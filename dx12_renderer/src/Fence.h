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

    HRESULT CPUSignal( uint64_t fenceValue );
    DWORD CPUWait( uint64_t fenceValue, uint64_t duration = 0xFFFFFFFFFFFFFFFF );
    HRESULT GPUSignal( ComPtr<ID3D12CommandQueue> commandQueue, uint64_t fenceValue );
    HRESULT GPUWait( ComPtr<ID3D12CommandQueue> commandQueue, uint64_t fenceValue );

    uint64_t getCompletedValue() const { return m_fence->GetCompletedValue(); }

private:
    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent;
};

