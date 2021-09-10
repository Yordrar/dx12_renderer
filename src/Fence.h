#pragma once

#include <cstdint>
#include <chrono>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class Fence
{
public:
    Fence( ComPtr<ID3D12Device> device );
    ~Fence();

    uint64_t signal( ComPtr<ID3D12CommandQueue> cmdQueue );
    void waitForValue( uint64_t fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds::max() );

private:
    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent;
    uint64_t m_fenceValue = 0;
};

