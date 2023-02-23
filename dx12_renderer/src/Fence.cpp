#include "Fence.h"

#include <pix3.h>

#include <Renderer.h>

Fence::Fence( wchar_t const* name )
{
    Renderer::device()->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) );

    m_fenceEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    m_fence->SetName( name );
}

Fence::~Fence()
{

}

uint64_t Fence::CPUSignal()
{
    m_fence->Signal( ++m_fenceValue );

    return m_fenceValue;
}

void Fence::CPUWait( uint64_t fenceValue, uint64_t duration )
{
    if ( m_fence->GetCompletedValue() < fenceValue )
    {
        m_fence->SetEventOnCompletion( fenceValue, m_fenceEvent );
        WaitForSingleObject( m_fenceEvent, static_cast<DWORD>( duration ) );
        PIXNotifyWakeFromFenceSignal( m_fenceEvent );
    }
}

uint64_t Fence::GPUSignal( ComPtr<ID3D12CommandQueue> commandQueue )
{
    commandQueue->Signal( m_fence.Get(), ++m_fenceValue );

    return m_fenceValue;
}

void Fence::GPUWait( ComPtr<ID3D12CommandQueue> commandQueue, uint64_t fenceValue )
{
    commandQueue->Wait( m_fence.Get(), fenceValue );
}