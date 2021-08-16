#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class Renderer
{
public:
    Renderer(HWND hWnd, RECT windowRect);
    ~Renderer();

    static constexpr UINT sc_numBackBuffers = 3;

private:

    HWND m_hWnd;
    RECT m_windowRect;

    ComPtr<ID3D12Device> m_device;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[ sc_numBackBuffers ];
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    UINT m_RTVDescriptorSize;
    ComPtr<ID3D12Resource> m_backBuffers[ sc_numBackBuffers ];
    UINT m_CurrentBackBufferIndex;
};

