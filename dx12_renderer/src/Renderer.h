#pragma once

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

    void setCurrentScene( Scene const& scene ) { m_currentScene = &scene; }

    void beginFrame();
    void submitRenderPass( RenderPass& pass );
    void endFrame();

    void waitForIdleGPU();

    static UINT getCurrentBackbufferIndex() { return s_currentBackBufferIndex; }
    static UINT getPreviousBackbufferIndex();
    static RECT getWindowRect() { return s_windowRect; }
    static ComPtr<ID3D12RootSignature> getRootSignature() { return s_rootSignature; }
    static uint64_t getTimestampFrequency() { return s_timestampFrequency; }

private:
    HWND m_hWnd;
    static RECT s_windowRect;

    static ComPtr<ID3D12Device2> s_device;
    static ComPtr<ID3D12RootSignature> s_rootSignature;
    static UINT s_currentBackBufferIndex;
    static uint64_t s_timestampFrequency;

    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource> m_backBuffers[ RendererConstants::sc_numBackBuffers ];
    std::unique_ptr<Fence> m_frameFence;
    uint64_t m_fenceValues[ RendererConstants::sc_numBackBuffers ];

    ComPtr<ID3D12CommandQueue> m_graphicsCmdQueue;
    ComPtr<ID3D12CommandQueue> m_computeCmdQueue;

    Scene const* m_currentScene;
};

