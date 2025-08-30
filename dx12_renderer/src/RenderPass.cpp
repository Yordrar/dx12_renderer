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
    , m_renderTarget( renderTarget )
    , m_depthStencilTarget( depthStencilTarget )
    , m_clearRenderTargetsBeforeRendering( false )
    , m_scissorRect( D3D12_RECT{} )
    , m_useCustomScissorRect( false )
    , m_profilerQueryIndex( -1 )
    , m_executionTimeInMilliseconds( 0 )
{
    memset(m_rtFormats.RTFormats, DXGI_FORMAT_UNKNOWN, sizeof(m_rtFormats.RTFormats));
    m_rtFormats.NumRenderTargets = 0;
    if (renderTarget.getType() != Descriptor::Type::InvalidView)
    {
        m_rtFormats.RTFormats[0] = renderTarget.getRTVDesc().Format;
        m_rtFormats.NumRenderTargets = 1;
    }

    m_dsFormat = depthStencilTarget.getDSVDesc().Format;
}

RenderPass::~RenderPass()
{

}

void RenderPass::record( Renderer& renderer, ComPtr<ID3D12GraphicsCommandList> commandList, Scene& scene, std::vector<Camera*> const& cameras )
{
    if (m_profilerQueryIndex == -1)
    {
        m_profilerQueryIndex = renderer.getProfiler().allocateQueryIndex();
    }

    Descriptor renderTarget;
    if (m_renderTarget.isValid())
    {
        renderTarget = m_renderTarget;
    }
    else
    {
        renderTarget = renderer.getCurrentBackbufferRTV();
        m_rtFormats.RTFormats[0] = renderTarget.getRTVDesc().Format;
        m_rtFormats.NumRenderTargets = 1;
    }

    static FLOAT clearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };

    if ( !m_passBuffer.isValid() && m_passBufferData.size() > 0 )
    {
        m_passBuffer = ResourceManager::it().createResource( ( m_name + L"_passBuffer" ).c_str(),
                                                                            CD3DX12_RESOURCE_DESC::Buffer( m_passBufferData.size() * sizeof( UINT ) ),
                                                                            D3D12_SUBRESOURCE_DATA{ m_passBufferData.data(), static_cast<LONG_PTR>( m_passBufferData.size() * sizeof( UINT ) ), 0 } );
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
        {
            .Format = DXGI_FORMAT_R32_TYPELESS,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        };
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Buffer.NumElements = static_cast<UINT>( m_passBufferData.size() );
        srvDesc.Buffer.StructureByteStride = 0;
        m_passBufferDescriptor = ResourceManager::it().getShaderResourceView( m_passBuffer, srvDesc );
    }

    renderer.getProfiler().startQuery( commandList.Get(), m_profilerQueryIndex );

    ResourceManager::it().copyResourcesToGPU( commandList );

    PIXBeginEvent( commandList.Get(), PIX_COLOR_DEFAULT, m_name.c_str() );

    // Set viewport
    D3D12_VIEWPORT viewport = CD3DX12_VIEWPORT(ResourceManager::it().getD3DResource(renderTarget.getResourceHandle()).Get() );
    commandList->RSSetViewports( 1, &viewport );

    /*ComPtr<ID3D12GraphicsCommandList5> commandList5;
    commandList.As(&commandList5);
    commandList5->RSSetShadingRate(D3D12_SHADING_RATE_2X2, nullptr);*/

    // Set scissor
    if (m_useCustomScissorRect)
    {
        commandList->RSSetScissorRects(1, &m_scissorRect);
    }
    else
    {
        D3D12_RECT scissorRect = renderer.getClientRect();
        commandList->RSSetScissorRects(1, &scissorRect);
    }

    // Set descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] =
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get(),
        DescriptorHeap::getDescriptorHeapSampler().getHeap().Get(),
    };
    commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

    // Set root signature
    commandList->SetGraphicsRootSignature(renderer.getRootSignature().Get());

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
    br.submitBarriers(commandList);

    if ( m_clearRenderTargetsBeforeRendering )
    {
        FLOAT const clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        commandList->ClearRenderTargetView(renderTarget.getDescriptorHandle(), clearColor, 0, nullptr);
        if ((m_depthStencilTarget.getDSVDesc().Flags & D3D12_DSV_FLAG_READ_ONLY_DEPTH) == 0)
        {
            commandList->ClearDepthStencilView(m_depthStencilTarget.getDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthStencilTarget.getDescriptorHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = renderTarget.getDescriptorHandle();
    commandList->OMSetRenderTargets( 1, &rtv, false, &dsv );

    // Record scenes
    std::wstring currentMaterialName;
    Material::PSODesc activePSODesc;
    Material::PSODesc currentPSODesc =
    {
        .m_passName = m_techniqueName,
        .m_depthStencilState = m_depthStencilState,
        .m_blendState = m_blendState,
        .m_rtFormats = m_rtFormats,
        .m_dsFormat = m_dsFormat
    };
    commandList->SetGraphicsRoot32BitConstant( 0, m_passBufferDescriptor.getDescriptorIndex(), 0 );
    for ( std::shared_ptr<Mesh> const& currentMesh : scene.getMeshes() )
    {
        for (Mesh::Submesh const& currentSubmesh : currentMesh->getSubmeshes())
        {
            if (currentSubmesh.m_vertexBuffer.get() == nullptr)
            {
                continue;
            }

            Material* currentMeshMaterial = currentSubmesh.m_material.get();

            commandList->SetGraphicsRoot32BitConstant(0, currentMeshMaterial->getMaterialBufferDescriptor().getDescriptorIndex(), 2);

            currentPSODesc.m_shaderFilepath = currentSubmesh.m_shaderFilepath,
            currentPSODesc.m_primitiveTopologyType = currentSubmesh.m_primitiveTopologyType;
            currentPSODesc.m_rasterizerState = currentSubmesh.m_rasterizerState;
                
            if (currentPSODesc != activePSODesc)
            {
                commandList->SetPipelineState(currentMeshMaterial->getPSO(currentPSODesc).Get());
                activePSODesc = currentPSODesc;
            }

            for (Descriptor const& descriptor : currentMeshMaterial->getResourceViews())
            {
                if (descriptor.getType() == Descriptor::Type::ShaderResourceView)
                {
                    br.recordBarrierTransition(descriptor.getResourceHandle(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                }
                else if (descriptor.getType() == Descriptor::Type::UnorderedAccessView)
                {
                    br.recordBarrierTransition(descriptor.getResourceHandle(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                }
            }

            for (Camera* camera : cameras)
            {
                br.recordBarrierTransition(camera->getGPUBufferResource(), D3D12_RESOURCE_STATE_COMMON);

                commandList->SetGraphicsRoot32BitConstant(0, camera->getCameraBufferDescriptor().getDescriptorIndex(), 1);

                br.submitBarriers(commandList);

                currentSubmesh.record(commandList);
            }
        }
    }

    PIXEndEvent( commandList.Get() );

    renderer.getProfiler().endQuery( commandList.Get(), m_profilerQueryIndex );
    m_executionTimeInMilliseconds = renderer.getProfiler().getResolvedQuery( m_profilerQueryIndex );
}

void RenderPass::addResourceView( Descriptor const& descriptor )
{
    m_passBufferData.push_back( descriptor.getDescriptorIndex() );
    m_passResources.push_back( descriptor );
}

void RenderPass::setScissorRect(D3D12_RECT const& rect)
{
    m_useCustomScissorRect = true;
    m_scissorRect = rect;
}

void RenderPass::setDepthWriteMask(D3D12_DEPTH_WRITE_MASK depthWriteMask)
{
    m_depthStencilState.DepthWriteMask = depthWriteMask;
}

void RenderPass::setDepthFunc(D3D12_COMPARISON_FUNC depthFunc)
{
    m_depthStencilState.DepthFunc = depthFunc;
}

void RenderPass::addFenceToSignal( Fence& fence )
{
    m_fencesToSignal.push_back( &fence );
}

void RenderPass::addFenceToWaitOn( Fence& fence )
{
    m_fencesToWaitOn.push_back( &fence );
}
