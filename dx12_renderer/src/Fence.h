#pragma once

#include <cstdint>
#include <limits>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class Fence
{
public:
    Fence( wchar_t const* name );
    ~Fence();

    uint64_t CPUSignal();
    void CPUWait( uint64_t fenceValue, uint64_t duration = 0xFFFFFFFFFFFFFFFF );
    uint64_t GPUSignal( ComPtr<ID3D12CommandQueue> commandQueue );
    void GPUWait( ComPtr<ID3D12CommandQueue> commandQueue, uint64_t fenceValue );

    uint64_t getNextSignalValue() const { return m_fenceValue + 1; }
    uint64_t getLastSignaledValue() const { return m_fenceValue; }
    uint64_t getCompletedValue() const { return m_fence->GetCompletedValue(); }

private:
    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent;
    uint64_t m_fenceValue = 0;
};

