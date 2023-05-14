#include "RenderPass.h"

#include <Renderer.h>
#include <Profiler.h>
#include <resource/ResourceManager.h>
#include <resource/Descriptor.h>
#include <geometry/MaterialManager.h>

RenderPass::RenderPass( wchar_t const* name,
                        wchar_t const* techniqueName,
                        Descriptor const* renderTarget,
                        Descriptor const* depthStencilTarget,
                        bool useBackbufferAsRenderTarget )
    : m_name( name )
    , m_techniqueName( techniqueName )
    , m_commandList( nullptr )
    , m_renderTarget( std::move( renderTarget ) )
    , m_depthStencilTarget( std::move( depthStencilTarget ) )
    , m_useBackbufferAsRenderTarget( useBackbufferAsRenderTarget )
    , m_scissorRect( Renderer::getWindowRect() )
    , m_profilerQueryIndex( Profiler::it().allocateQueryIndex() )
    , m_executionTimeInMilliseconds( 0 )
    , m_passBuffer( nullptr )
    , m_passResourceIndicesBuffer( nullptr )
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

void RenderPass::record( Scene const& scene, std::vector<Camera*> const& cameras )
{
    // Reset command list and allocator
    ComPtr<ID3D12CommandAllocator> currentCommandAllocator = m_commandAllocators[ Renderer::getCurrentBackbufferIndex() ];
    currentCommandAllocator->Reset();
    m_commandList->Reset( currentCommandAllocator.Get(), nullptr );

    static FLOAT clearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };

    if ( m_useBackbufferAsRenderTarget )
    {
        m_renderTarget = Renderer::getCurrentBackbufferRTV();
    }

    if ( !m_passResourceIndicesBuffer )
    {
        m_passResourceIndicesBuffer = ResourceManager::it().createResource( ( m_name + L"_passResourceIndicesBuffer" ).c_str(),
                                                                            CD3DX12_RESOURCE_DESC::Buffer( std::max( m_passResourceIndicesBufferData.size() * sizeof( UINT ), 1Ui64 ) ),
                                                                            D3D12_SUBRESOURCE_DATA{ m_passResourceIndicesBufferData.data(), static_cast<LONG_PTR>( m_passResourceIndicesBufferData.size() * sizeof( UINT ) ), 0 } );
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
        {
            .Format = DXGI_FORMAT_R32_TYPELESS,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        };
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Buffer.NumElements = static_cast<UINT>( std::max( m_passResourceIndicesBufferData.size(), 1Ui64 ) );
        srvDesc.Buffer.StructureByteStride = 0;
        m_passBufferData.passResourceIndicesBufferIndex = m_passResourceIndicesBuffer->getShaderResourceView( srvDesc )->getDescriptorIndex();
    }
    if ( !m_passBuffer )
    {
        m_passBuffer = ResourceManager::it().createResource( ( m_name + L"_passBuffer" ).c_str(),
                                                             CD3DX12_RESOURCE_DESC::Buffer( std::max( sizeof( m_passBufferData ), 1Ui64 ) ),
                                                             D3D12_SUBRESOURCE_DATA{ &m_passBufferData, static_cast<LONG_PTR>( sizeof( m_passBufferData ) ), 0 } );
    }

    Profiler::it().startQuery( m_commandList.Get(), m_profilerQueryIndex );

    ResourceManager::it().copyResourcesToGPU( m_commandList );

    PIXBeginEvent( m_commandList.Get(), PIX_COLOR_DEFAULT, m_name.c_str() );

    // Set viewport
    if ( m_renderTarget )
    {
        D3D12_VIEWPORT viewport = CD3DX12_VIEWPORT( m_renderTarget->getResource()->getD3DResource().Get() );
        m_commandList->RSSetViewports( 1, &viewport );
    }
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
    std::vector<D3D12_RESOURCE_BARRIER> barriers;
    if ( m_renderTarget && m_renderTarget->getResource()->getResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET )
    {
        D3D12_RESOURCE_BARRIER renderTargetBarrier = m_renderTarget->getResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_RENDER_TARGET );
        barriers.push_back( renderTargetBarrier );
    }
    if ( m_depthStencilTarget )
    {
        if ( m_depthStencilTarget->getDSVFlags() == D3D12_DSV_FLAG_NONE && m_depthStencilTarget->getResource()->getResourceState() != D3D12_RESOURCE_STATE_DEPTH_WRITE )
        {
            D3D12_RESOURCE_BARRIER depthStencilBarrier = m_depthStencilTarget->getResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_DEPTH_WRITE );
            barriers.push_back( depthStencilBarrier );
        }
        else if ( m_depthStencilTarget->getDSVFlags() == D3D12_DSV_FLAG_READ_ONLY_DEPTH && m_depthStencilTarget->getResource()->getResourceState() != D3D12_RESOURCE_STATE_DEPTH_READ )
        {
            D3D12_RESOURCE_BARRIER depthStencilBarrier = m_depthStencilTarget->getResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_DEPTH_READ );
            barriers.push_back( depthStencilBarrier );
        }
    }
    for ( Descriptor const& descriptor : m_passResources )
    {
        if ( descriptor.getResource()->getResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE )
        {
            D3D12_RESOURCE_BARRIER barrier = descriptor.getResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
            barriers.push_back( barrier );
        }
    }
    if ( barriers.size() > 0 )
    {
        m_commandList->ResourceBarrier( static_cast<UINT>( barriers.size() ), barriers.data() );
    }

    if ( m_renderTarget )
    {
        m_commandList->ClearRenderTargetView( m_renderTarget->getView(), clearColor, 0, nullptr );
    }
    if ( m_depthStencilTarget && m_depthStencilTarget->getResource()->getResourceState() == D3D12_RESOURCE_STATE_DEPTH_WRITE )
    {
        m_commandList->ClearDepthStencilView( m_depthStencilTarget->getView(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthStencilTarget->getView();
    if ( m_renderTarget )
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_renderTarget->getView();
        m_commandList->OMSetRenderTargets( 1, &rtv, false, &dsv );
    }
    else
    {
        m_commandList->OMSetRenderTargets( 0, nullptr, false, &dsv );
    }

    // Record scenes
    std::wstring currentMaterialName;
    m_commandList->SetGraphicsRootConstantBufferView( 0, m_passBuffer->getGPUVirtualAddress() );
    for ( std::shared_ptr<Mesh> const& currentMesh : scene.getMeshes() )
    {
        if ( !currentMesh->hasVertexBuffer() )
        {
            continue;
        }

        for ( Camera* camera : cameras )
        {
            std::vector<D3D12_RESOURCE_BARRIER> barriers;
            if ( camera->getGPUBufferResource()->getResourceState() != D3D12_RESOURCE_STATE_COMMON )
            {
                D3D12_RESOURCE_BARRIER barrier = camera->getGPUBufferResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_COMMON );
                barriers.push_back( barrier );
            }
            m_commandList->SetGraphicsRootConstantBufferView( 1, camera->getGPUBufferResource()->getGPUVirtualAddress() );
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
                    m_commandList->SetGraphicsRootConstantBufferView( 2, currentMeshMaterial->getMaterialBufferResource()->getGPUVirtualAddress() );
                    m_commandList->SetPipelineState( currentMeshMaterial->getPSOForTechnique( m_techniqueName.c_str() ).Get() );
                    for ( Descriptor const* descriptor : currentMeshMaterial->getResourceViews() )
                    {
                        if ( descriptor->getType() == Descriptor::Type::ShaderResourceView &&
                             descriptor->getResource()->getResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE )
                        {
                            D3D12_RESOURCE_BARRIER barrier = descriptor->getResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
                            barriers.push_back( barrier );
                        }
                        else if ( descriptor->getType() == Descriptor::Type::UnorderedAccessView &&
                                  descriptor->getResource()->getResourceState() != D3D12_RESOURCE_STATE_UNORDERED_ACCESS )
                        {
                            D3D12_RESOURCE_BARRIER barrier = descriptor->getResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_UNORDERED_ACCESS );
                            barriers.push_back( barrier );
                        }
                    }
                    currentMaterialName = currentMeshMaterialName;
                }
                else
                {
                    continue;
                }
            }

            if ( barriers.size() > 0 )
            {
                m_commandList->ResourceBarrier( static_cast<UINT>( barriers.size() ), barriers.data() );
            }

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
    m_passResourceIndicesBufferData.push_back( descriptor.getDescriptorIndex() );
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
