#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <memory>

#include "Fence.h"

class Renderer
{
public:
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
    };

    struct RenderContext
    {
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        PipelineStateStream& m_pipelineState;
        ComPtr<ID3D12DescriptorHeap> m_cameraDescriptors;
    };

    Renderer(HWND hWnd, RECT windowRect);
    ~Renderer();

    static ComPtr<ID3D12Device2> device() { return m_device; }

    void renderScene();

    static constexpr UINT sc_numBackBuffers = 3;

private:

    HWND m_hWnd;
    RECT m_windowRect;

    static ComPtr<ID3D12Device2> m_device;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[ sc_numBackBuffers ];
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    std::unique_ptr<Fence> m_fence;
    ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    UINT m_rtvDescriptorSize;
    ComPtr<ID3D12Resource> m_backBuffers[ sc_numBackBuffers ];
    UINT m_currentBackBufferIndex;
};

