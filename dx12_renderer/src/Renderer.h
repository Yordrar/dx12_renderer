#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>
#include <thread>
#include <algorithm>

#include <RendererConstants.h>
#include <RenderPass.h>
#include <Fence.h>

class Scene;

class Renderer
{
public:
    Renderer(HWND hWnd, RECT windowRect);
    ~Renderer() = default;

    static ComPtr<ID3D12Device2> device() { return s_device; }

    void beginFrame();
    void submitPass( RenderPass& pass );
    void endFrame();

    void waitForIdleGPU();

    static UINT getCurrentBackbufferIndex() { return s_currentBackBufferIndex; }
    static UINT getPreviousBackbufferIndex() { return std::clamp( s_currentBackBufferIndex - 1, 0u, RendererConstants::sc_numBackBuffers - 1 ); }
    static RECT getWindowRect() { return s_windowRect; }
    static ComPtr<ID3D12RootSignature> getRootSignature() { return s_rootSignature; }
    static uint64_t getTimestampFrequency() { return s_timestampFrequency; }

private:
    HWND m_hWnd;
    static RECT s_windowRect;

    static ComPtr<ID3D12Device2> s_device;
    static UINT s_currentBackBufferIndex;
    static uint64_t s_timestampFrequency;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource> m_backBuffers[ RendererConstants::sc_numBackBuffers ];
    std::unique_ptr<Fence> m_frameFence;
    uint64_t m_fenceValues[ RendererConstants::sc_numBackBuffers ];

    ComPtr<ID3D12CommandQueue> m_graphicsCmdQueue;
    ComPtr<ID3D12CommandQueue> m_computeCmdQueue;
    ComPtr<ID3D12CommandQueue> m_copyCmdQueue;

    static ComPtr<ID3D12RootSignature> s_rootSignature;
};

