#include "RenderPass.h"

#include <pix3.h>

#include <Renderer.h>
#include <Profiler.h>
#include <resource/ResourceManager.h>
#include <resource/Descriptor.h>
#include <geometry/PSOManager.h>
#include <geometry/IGeometry.h>

RenderPass::RenderPass( wchar_t const* name,
                        wchar_t const* techniqueName,
                        wchar_t const* renderTargetName,
                        wchar_t const* depthStencilTargetName )
    : m_name( name )
    , m_techniqueName( techniqueName )
    , m_renderTargetName( renderTargetName )
    , m_commandList( nullptr )
    , m_renderTarget( nullptr )
    , m_depthStencilTarget( nullptr )
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

void RenderPass::record()
{
    // Reset command list and allocator
    ComPtr<ID3D12CommandAllocator> currentCommandAllocator = m_commandAllocators[ Renderer::getCurrentRecordingIndex() ];
    currentCommandAllocator->Reset();
    m_commandList->Reset( currentCommandAllocator.Get(), nullptr );

    Profiler::it().startQuery( m_commandList.Get(), m_profilerQueryIndex );

    ResourceManager::it().copyResourcesToGPU( m_commandList );

    PIXBeginEvent( m_commandList.Get(), PIX_COLOR_DEFAULT, m_name.c_str() );

    // Set viewport
    CD3DX12_VIEWPORT viewport( ResourceManager::it().getCurrentBackbufferResource()->getResource().Get() );
    m_commandList->RSSetViewports( 1, &viewport );
    D3D12_RECT const& scissorRect = Renderer::getWindowRect();
    m_commandList->RSSetScissorRects( 1, &scissorRect );

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
    m_commandList->SetGraphicsRootDescriptorTable( 4, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 5, DescriptorHeap::getDescriptorHeapSampler().getHeap()->GetGPUDescriptorHandleForHeapStart() );

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
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_renderTarget->getRenderTargetView()->getView();
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthStencilTarget->getDepthStencilView()->getView();
        m_commandList->OMSetRenderTargets( 1, &rtv, false, &dsv );
    }
    else
    {
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthStencilTarget->getDepthStencilView()->getView();
        m_commandList->OMSetRenderTargets( 0, nullptr, false, &dsv );
    }

    // Record scenes
    for ( Scene* scene : m_scenes )
    {
        scene->record( m_techniqueName.c_str(), m_commandList );
    }

    if ( m_renderTarget && m_renderTarget->getName().rfind( L"backbuffer", 0 ) == 0 )
    {
        std::optional<CD3DX12_RESOURCE_BARRIER> renderTargetBarrier = m_renderTarget->getTransitionBarrier( D3D12_RESOURCE_STATE_PRESENT );
        if ( renderTargetBarrier.has_value() )
        {
            m_commandList->ResourceBarrier( 1, &renderTargetBarrier.value() );
        }
    }

    PIXEndEvent( m_commandList.Get() );

    Profiler::it().endQuery( m_commandList.Get(), m_profilerQueryIndex );
    m_executionTimeInMilliseconds = Profiler::it().getResolvedQuery( m_profilerQueryIndex );

    m_commandList->Close();
}
