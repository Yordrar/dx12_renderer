#include "RenderPass.h"

#include <pix3.h>

#include <Renderer.h>
#include <resource/ResourceManager.h>
#include <geometry/PSOManager.h>
#include <geometry/IGeometry.h>

RenderPass::RenderPass( std::string name,
                        std::string techniqueName,
                        std::string renderTargetName,
                        std::string depthStencilTargetName )
    : m_name( name )
    , m_techniqueName( techniqueName )
{
    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i )
    {
        Renderer::device()->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_commandAllocators[ i ] ) );
    }

    Renderer::device()->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr, nullptr, IID_PPV_ARGS( &m_commandList ) );
    m_commandList->Close();

    m_renderTarget = ResourceManager::it().getResource<Texture>( renderTargetName );
    assert( m_renderTarget );

    m_depthStencilTarget = ResourceManager::it().getResource<Texture>( depthStencilTargetName );
    assert( m_depthStencilTarget );
}

RenderPass::~RenderPass()
{

}

void RenderPass::record( Scene& scene )
{
    PIXBeginEvent( PIX_COLOR_DEFAULT, m_name.c_str() );
    // Reset command list and allocator
    ComPtr<ID3D12CommandAllocator> currentCommandAllocator = m_commandAllocators[ Renderer::getCurrentFrameIndex() ];
    currentCommandAllocator->Reset();
    m_commandList->Reset( currentCommandAllocator.Get(), nullptr );

    // Set viewport
    m_commandList->RSSetViewports( 1, &CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( Renderer::getWindowRect().right - Renderer::getWindowRect().left ), static_cast<float>( Renderer::getWindowRect().bottom - Renderer::getWindowRect().top ) ) );
    m_commandList->RSSetScissorRects( 1, &CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) );

    // Set root signature
    m_commandList->SetGraphicsRootSignature( Renderer::getRootSignature().Get() );

    // Descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] =
    {
        ResourceManager::it().getCbvSrvUavDescriptorHeap().getHeap().Get(),
        ResourceManager::it().getSamplerDescriptorHeap().getHeap().Get(),
    };
    m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );
    m_commandList->SetGraphicsRootDescriptorTable( 3, ResourceManager::it().getCbvSrvUavDescriptorHeap().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 4, ResourceManager::it().getCbvSrvUavDescriptorHeap().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 5, ResourceManager::it().getCbvSrvUavDescriptorHeap().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 6, ResourceManager::it().getSamplerDescriptorHeap().getHeap()->GetGPUDescriptorHandleForHeapStart() );

    // Resource barriers
    m_renderTarget->transitionToState( m_commandList, D3D12_RESOURCE_STATE_RENDER_TARGET );
    m_depthStencilTarget->transitionToState( m_commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE );

    // Clear the render targets
    static FLOAT clearColor[ 4 ] = { 0.4f, 0.6f, 0.9f, 1.0f };
    m_commandList->ClearRenderTargetView( m_renderTarget->getDescriptor(), clearColor, 0, nullptr );
    m_commandList->ClearDepthStencilView( m_depthStencilTarget->getDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0.0f, 0, nullptr );

    m_commandList->OMSetRenderTargets( 1, &m_renderTarget->getDescriptor(), false, &m_depthStencilTarget->getDescriptor() );

    // Draw geometry
    PSOManager::PipelineStateStream pipelineState;
    pipelineState.m_rasterizer = CD3DX12_RASTERIZER_DESC();
    std::vector<DXGI_FORMAT> rtFormats;
    rtFormats.push_back( m_renderTarget->getFormat() );
    pipelineState.m_rtFormats = CD3DX12_RT_FORMAT_ARRAY( rtFormats.data(), rtFormats.size() );
    pipelineState.m_dsFormat = m_depthStencilTarget->getFormat();
    pipelineState.m_rootSignature = Renderer::getRootSignature().Get();

    std::vector< std::shared_ptr<IGeometry> > geometry;
    scene.getGeometryForTechnique( m_techniqueName, geometry );
    for ( std::shared_ptr<IGeometry>& g : geometry )
    {
        g->recordRenderPass( m_techniqueName, m_commandList, pipelineState );
    }

    m_renderTarget->transitionToState( m_commandList, D3D12_RESOURCE_STATE_PRESENT );

    m_commandList->Close();
    PIXEndEvent();
}
