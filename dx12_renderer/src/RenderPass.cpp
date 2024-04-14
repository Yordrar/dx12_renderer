#include "RenderPass.h"

#include <Renderer.h>
#include <Profiler.h>
#include <BarrierRecorder.h>
#include <resource/ResourceManager.h>
#include <resource/Descriptor.h>
#include <geometry/MaterialManager.h>

RenderPass::RenderPass( wchar_t const* name,
                        wchar_t const* techniqueName,
                        Descriptor renderTarget,
                        Descriptor depthStencilTarget )
    : m_name( name )
    , m_techniqueName( techniqueName )
    , m_commandList( nullptr )
    , m_renderTarget( renderTarget )
    , m_depthStencilTarget( depthStencilTarget )
    , m_scissorRect( Renderer::getClientRect() )
    , m_profilerQueryIndex( Profiler::it().allocateQueryIndex() )
    , m_executionTimeInMilliseconds( 0 )
{
    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i )
    {
        Renderer::device()->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_commandAllocators[ i ] ) );
    }

    HRESULT result = Renderer::device()->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[ 0 ].Get(), nullptr, IID_PPV_ARGS( &m_commandList ) );
    m_commandList->Close();
    std::wstring commandListName = m_name + L"_commandList";
    m_commandList->SetName( commandListName.c_str() );
}

RenderPass::~RenderPass()
{

}

void RenderPass::record( Scene& scene, std::vector<Camera*> const& cameras )
{
    // Reset command list and allocator
    ComPtr<ID3D12CommandAllocator> currentCommandAllocator = m_commandAllocators[ Renderer::getCurrentBackbufferIndex() ];
    currentCommandAllocator->Reset();
    m_commandList->Reset( currentCommandAllocator.Get(), nullptr );

    Descriptor renderTarget;
    if (m_renderTarget.isValid())
    {
        renderTarget = m_renderTarget;
    }
    else
    {
        renderTarget = Renderer::getCurrentBackbufferRTV();
    }

    static FLOAT clearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };

    if ( !m_passBuffer.isValid() )
    {
        m_passBuffer = ResourceManager::it().createResource( ( m_name + L"_passBuffer" ).c_str(),
                                                                            CD3DX12_RESOURCE_DESC::Buffer( std::max(m_passBufferData.size() * sizeof( UINT ), 1Ui64 ) ),
                                                                            D3D12_SUBRESOURCE_DATA{ m_passBufferData.data(), static_cast<LONG_PTR>( m_passBufferData.size() * sizeof( UINT ) ), 0 } );
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
        {
            .Format = DXGI_FORMAT_R32_TYPELESS,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        };
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Buffer.NumElements = static_cast<UINT>( std::max( m_passBufferData.size(), 1Ui64 ) );
        srvDesc.Buffer.StructureByteStride = 0;
        m_passBufferDescriptor = ResourceManager::it().getShaderResourceView( m_passBuffer, srvDesc );
    }

    Profiler::it().startQuery( m_commandList.Get(), m_profilerQueryIndex );

    ResourceManager::it().copyResourcesToGPU( m_commandList );

    PIXBeginEvent( m_commandList.Get(), PIX_COLOR_DEFAULT, m_name.c_str() );

    // Set viewport
    D3D12_VIEWPORT viewport = CD3DX12_VIEWPORT(ResourceManager::it().getD3DResource(renderTarget.getResourceHandle()).Get() );
    m_commandList->RSSetViewports( 1, &viewport );
    m_commandList->RSSetScissorRects( 1, &m_scissorRect );

    // Set descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] =
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get(),
        DescriptorHeap::getDescriptorHeapSampler().getHeap().Get(),
    };
    m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

    // Set root signature
    m_commandList->SetGraphicsRootSignature(Renderer::getRootSignature().Get());

    // Clear and set render targets
    BarrierRecorder br;
    br.recordBarrierTransition(renderTarget.getResourceHandle(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    if (m_depthStencilTarget.getDSVDesc().Flags & D3D12_DSV_FLAG_READ_ONLY_DEPTH)
    {
        br.recordBarrierTransition(m_depthStencilTarget.getResourceHandle(), D3D12_RESOURCE_STATE_DEPTH_READ);
    }
    else
    {
        br.recordBarrierTransition(m_depthStencilTarget.getResourceHandle(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    for ( Descriptor const& descriptor : m_passResources )
    {
        br.recordBarrierTransition(descriptor.getResourceHandle(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
    br.submitBarriers(m_commandList);

    if ( (m_depthStencilTarget.getDSVDesc().Flags & D3D12_DSV_FLAG_READ_ONLY_DEPTH) == 0 )
    {
        m_commandList->ClearDepthStencilView( m_depthStencilTarget.getDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthStencilTarget.getDescriptorHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = renderTarget.getDescriptorHandle();
    m_commandList->OMSetRenderTargets( 1, &rtv, false, &dsv );

    // Record scenes
    std::wstring currentMaterialName;
    m_commandList->SetGraphicsRoot32BitConstant( 0, m_passBufferDescriptor.getDescriptorIndex(), 0 );
    for ( std::shared_ptr<Mesh> const& currentMesh : scene.getMeshes() )
    {
        if ( !currentMesh->hasVertexBuffer() )
        {
            continue;
        }

        for ( Camera* camera : cameras )
        {
            br.recordBarrierTransition(camera->getGPUBufferResource(), D3D12_RESOURCE_STATE_COMMON);
            m_commandList->SetGraphicsRoot32BitConstant(0, camera->getCameraBufferDescriptor().getDescriptorIndex(), 1);
            if ( currentMesh->isAABBValid() && !camera->isAABBVisible( currentMesh->getAABB() ) )
            {
                continue;
            }

            std::wstring const& currentMeshMaterialName = currentMesh->getMaterialName();
            if ( currentMaterialName != currentMeshMaterialName )
            {
                Material* currentMeshMaterial = MaterialManager::it().getMaterial( currentMeshMaterialName.c_str() );
                if ( currentMeshMaterial && currentMeshMaterial->hasTechnique( m_techniqueName.c_str() ) )
                {
                    m_commandList->SetGraphicsRoot32BitConstant( 0, currentMeshMaterial->getMaterialBufferDescriptor().getDescriptorIndex(), 2 );
                    m_commandList->SetPipelineState( currentMeshMaterial->getPSOForTechnique( m_techniqueName.c_str() ).Get() );
                    for ( Descriptor const& descriptor : currentMeshMaterial->getResourceViews() )
                    {
                        if ( descriptor.getType() == Descriptor::Type::ShaderResourceView )
                        {
                            br.recordBarrierTransition(descriptor.getResourceHandle(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                        }
                        else if ( descriptor.getType() == Descriptor::Type::UnorderedAccessView )
                        {
                            br.recordBarrierTransition(descriptor.getResourceHandle(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                        }
                    }
                    currentMaterialName = currentMeshMaterialName;
                }
                else
                {
                    continue;
                }
            }

            br.submitBarriers( m_commandList );

            currentMesh->record( m_commandList );
        }
    }

    PIXEndEvent( m_commandList.Get() );

    Profiler::it().endQuery( m_commandList.Get(), m_profilerQueryIndex );
    m_executionTimeInMilliseconds = Profiler::it().getResolvedQuery( m_profilerQueryIndex );

    m_commandList->Close();
}

void RenderPass::addResourceView( Descriptor const& descriptor )
{
    m_passBufferData.push_back( descriptor.getDescriptorIndex() );
    m_passResources.push_back( descriptor );
}

void RenderPass::addComputePassToWaitOn( ComputePass* computePass )
{
    m_computePassesToWaitOn.push_back( computePass );
    m_computePassFenceCounters.push_back( 0 );
}

void RenderPass::waitOnComputePasses( ComPtr<ID3D12CommandQueue> cmdQueue, std::vector<ComputePass*> const& submittedComputePasses )
{
    for ( size_t i = 0; i < m_computePassesToWaitOn.size(); ++i )
    {
        if ( std::find( submittedComputePasses.begin(), submittedComputePasses.end(), m_computePassesToWaitOn[ i ] ) != submittedComputePasses.end() )
        {
            m_computePassesToWaitOn[ i ]->getFence().GPUWait( cmdQueue, ++m_computePassFenceCounters[ i ] );
        }
    }
}
