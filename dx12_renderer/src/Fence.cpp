#include "Fence.h"

Fence::Fence( ComPtr<ID3D12Device> device )
{
    device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) );

    m_fenceEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
}

Fence::~Fence()
{

}

uint64_t Fence::signal( ComPtr<ID3D12CommandQueue> cmdQueue )
{
    cmdQueue->Signal( m_fence.Get(), ++m_fenceValue );

    return m_fenceValue;
}

void Fence::waitForValue( uint64_t fenceValue, std::chrono::milliseconds duration )
{
    if ( m_fence->GetCompletedValue() < fenceValue )
    {
        m_fence->SetEventOnCompletion( fenceValue, m_fenceEvent );
        WaitForSingleObject( m_fenceEvent, static_cast<DWORD>( duration.count() ) );
    }
}
