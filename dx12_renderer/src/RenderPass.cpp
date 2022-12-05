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
    std::string commandListName = name + "_commandList";
    m_commandList->SetName( std::wstring( commandListName.begin(), commandListName.end() ).c_str() );

    m_renderTarget = ResourceManager::it().getResource<Texture>( renderTargetName );

    m_depthStencilTarget = ResourceManager::it().getResource<Texture>( depthStencilTargetName );
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

    PIXBeginEvent( m_commandList.Get(), PIX_COLOR_DEFAULT, m_name.c_str() );

    // Set viewport
    m_commandList->RSSetViewports( 1, &CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( Renderer::getWindowRect().right - Renderer::getWindowRect().left ), static_cast<float>( Renderer::getWindowRect().bottom - Renderer::getWindowRect().top ) ) );
    m_commandList->RSSetScissorRects( 1, &CD3DX12_RECT( 0, 0, static_cast<float>( Renderer::getWindowRect().right - Renderer::getWindowRect().left ), static_cast<float>( Renderer::getWindowRect().bottom - Renderer::getWindowRect().top ) ) );

    // Set root signature
    m_commandList->SetGraphicsRootSignature( Renderer::getRootSignature().Get() );

    // Set descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] =
    {
        ResourceManager::it().getCbvSrvUavDescriptorHeap().getHeap().Get(),
        ResourceManager::it().getSamplerDescriptorHeap().getHeap().Get(),
    };
    m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

    // Set descriptor tables in root signature
    m_commandList->SetGraphicsRootDescriptorTable( 2, ResourceManager::it().getCbvSrvUavDescriptorHeap().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 3, ResourceManager::it().getCbvSrvUavDescriptorHeap().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 4, ResourceManager::it().getSamplerDescriptorHeap().getHeap()->GetGPUDescriptorHandleForHeapStart() );

    // Clear and set render targets
    static FLOAT clearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptor = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilTargetDescriptor = nullptr;
    if ( m_renderTargetName == "backbuffer" )
    {
        m_renderTarget = Renderer::getCurrentBackbuffer();
    }

    if ( m_renderTarget )
    {
        m_renderTarget->transitionToState( m_commandList, D3D12_RESOURCE_STATE_RENDER_TARGET );
        m_commandList->ClearRenderTargetView( m_renderTarget->getDescriptor(), clearColor, 0, nullptr );
        renderTargetDescriptor = &m_renderTarget->getDescriptor();
    }

    if ( m_depthStencilTarget )
    {
        m_depthStencilTarget->transitionToState( m_commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE );
        m_commandList->ClearDepthStencilView( m_depthStencilTarget->getDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0.0f, 0, nullptr );
        depthStencilTargetDescriptor = &m_depthStencilTarget->getDescriptor();
    }
    m_commandList->OMSetRenderTargets( renderTargetDescriptor ? 1 : 0, renderTargetDescriptor, false, depthStencilTargetDescriptor );

    // Set initial pipeline state
    PSOManager::PipelineStateStream pipelineState;
    CD3DX12_RASTERIZER_DESC rasterizerDesc( CD3DX12_DEFAULT{} );
    rasterizerDesc.FrontCounterClockwise = true;
    pipelineState.m_rasterizer = rasterizerDesc;

    std::vector<DXGI_FORMAT> rtFormats;
    m_renderTarget ? rtFormats.push_back( m_renderTarget->getFormat() ) : rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    rtFormats.push_back( DXGI_FORMAT_UNKNOWN );
    pipelineState.m_rtFormats = CD3DX12_RT_FORMAT_ARRAY( rtFormats.data(), m_renderTarget ? 1 : 0 );

    pipelineState.m_dsFormat = m_depthStencilTarget ? m_depthStencilTarget->getFormat() : DXGI_FORMAT_UNKNOWN;

    pipelineState.m_rootSignature = Renderer::getRootSignature().Get();

    // Record scene
    scene.record( m_techniqueName, m_commandList, pipelineState );

    if ( m_renderTarget )
    {
        m_renderTarget->transitionToState( m_commandList, D3D12_RESOURCE_STATE_PRESENT );
    }

    PIXEndEvent( m_commandList.Get() );

    m_commandList->Close();
}
