#include "RenderPass.h"

#include <pix3.h>

#include <Renderer.h>
#include <resource/ResourceManager.h>
#include <resource/Descriptor.h>
#include <geometry/PSOManager.h>
#include <geometry/IGeometry.h>

RenderPass::RenderPass( std::wstring name,
                        std::wstring techniqueName,
                        std::wstring renderTargetName,
                        std::wstring depthStencilTargetName )
    : m_name( name )
    , m_techniqueName( techniqueName )
    , m_renderTargetName( renderTargetName )
    , m_commandList( nullptr )
    , m_renderTarget( nullptr )
    , m_depthStencilTarget( nullptr )
{
    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i )
    {
        Renderer::device()->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_commandAllocators[ i ] ) );
    }

    HRESULT result = Renderer::device()->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[ 0 ].Get(), nullptr, IID_PPV_ARGS( &m_commandList ) );
    m_commandList->Close();
    std::wstring commandListName = name + L"_commandList";
    m_commandList->SetName( commandListName.c_str() );

    if ( renderTargetName == L"backbuffer" )
    {
        m_renderTarget = ResourceManager::it().getCurrentBackbufferResource();
    }
    else
    {
        m_renderTarget = ResourceManager::it().getResource( renderTargetName );
    }

    m_depthStencilTarget = ResourceManager::it().getResource( depthStencilTargetName );
}

RenderPass::~RenderPass()
{

}

void RenderPass::record( Scene& scene )
{
    // Reset command list and allocator
    ComPtr<ID3D12CommandAllocator> currentCommandAllocator = m_commandAllocators[ Renderer::getCurrentBackbufferIndex() ];
    currentCommandAllocator->Reset();
    m_commandList->Reset( currentCommandAllocator.Get(), nullptr );

    ResourceManager::it().copyResourcesToGPU( m_commandList );

    PIXBeginEvent( m_commandList.Get(), PIX_COLOR_DEFAULT, m_name.c_str() );

    // Set viewport
    m_commandList->RSSetViewports( 1, &CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( Renderer::getWindowRect().right - Renderer::getWindowRect().left ), static_cast<float>( Renderer::getWindowRect().bottom - Renderer::getWindowRect().top ) ) );
    m_commandList->RSSetScissorRects( 1, &CD3DX12_RECT( 0, 0, Renderer::getWindowRect().right - Renderer::getWindowRect().left, Renderer::getWindowRect().bottom - Renderer::getWindowRect().top ) );

    // Set root signature
    m_commandList->SetGraphicsRootSignature( Renderer::getRootSignature().Get() );

    // Set descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] =
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get(),
        DescriptorHeap::getDescriptorHeapSampler().getHeap().Get(),
    };
    m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

    // Set descriptor tables in root signature
    m_commandList->SetGraphicsRootDescriptorTable( 2, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 3, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 4, DescriptorHeap::getDescriptorHeapSampler().getHeap()->GetGPUDescriptorHandleForHeapStart() );

    // Clear and set render targets
    static FLOAT clearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if ( m_renderTargetName == L"backbuffer" )
    {
        m_renderTarget = ResourceManager::it().getCurrentBackbufferResource();
    }

    std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
    if ( m_renderTarget )
    {
        std::optional<CD3DX12_RESOURCE_BARRIER> renderTargetBarrier = m_renderTarget->getTransitionBarrier( D3D12_RESOURCE_STATE_RENDER_TARGET );
        if ( renderTargetBarrier.has_value() )
        {
            barriers.push_back( renderTargetBarrier.value() );
        }
    }
    if ( m_depthStencilTarget )
    {
        std::optional<CD3DX12_RESOURCE_BARRIER> depthStencilBarrier = m_depthStencilTarget->getTransitionBarrier( D3D12_RESOURCE_STATE_DEPTH_WRITE );
        if ( depthStencilBarrier.has_value() )
        {
            barriers.push_back( depthStencilBarrier.value() );
        }
    }
    if ( barriers.size() > 0 )
    {
        m_commandList->ResourceBarrier( static_cast<UINT>( barriers.size() ), barriers.data() );
    }
    if ( m_renderTarget )
    {
        m_commandList->ClearRenderTargetView( m_renderTarget->getRenderTargetView()->getView(), clearColor, 0, nullptr );
    }
    if ( m_depthStencilTarget )
    {
        m_commandList->ClearDepthStencilView( m_depthStencilTarget->getDepthStencilView()->getView(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
    }

    if ( m_renderTarget )
    {
        m_commandList->OMSetRenderTargets( 1, &m_renderTarget->getRenderTargetView()->getView(), false, &m_depthStencilTarget->getDepthStencilView()->getView() );
    }
    else
    {
        m_commandList->OMSetRenderTargets( 0, nullptr, false, &m_depthStencilTarget->getDepthStencilView()->getView() );
    }

    // Set initial pipeline state
    PSOManager::PipelineStateStream pipelineState;
    CD3DX12_RASTERIZER_DESC rasterizerDesc( CD3DX12_DEFAULT{} );
    rasterizerDesc.FrontCounterClockwise = true;
    pipelineState.m_rasterizerState = rasterizerDesc;

    std::vector<DXGI_FORMAT> rtFormats;
    m_renderTarget ? rtFormats.push_back( m_renderTarget->getResourceDesc().Format ) : rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    pipelineState.m_rtFormats = CD3DX12_RT_FORMAT_ARRAY( rtFormats.data(), m_renderTarget ? 1 : 0 );

    pipelineState.m_dsFormat = m_depthStencilTarget ? m_depthStencilTarget->getResourceDesc().Format : DXGI_FORMAT_UNKNOWN;

    pipelineState.m_rootSignature = Renderer::getRootSignature().Get();

    // Record scene
    scene.record( m_techniqueName, m_commandList, pipelineState );

    if ( m_renderTarget && m_renderTarget->getName().rfind( L"backbuffer", 0 ) == 0 )
    {
        std::optional<CD3DX12_RESOURCE_BARRIER> renderTargetBarrier = m_renderTarget->getTransitionBarrier( D3D12_RESOURCE_STATE_PRESENT );
        if ( renderTargetBarrier.has_value() )
        {
            m_commandList->ResourceBarrier( 1, &renderTargetBarrier.value() );
        }
    }

    PIXEndEvent( m_commandList.Get() );

    m_commandList->Close();
}
