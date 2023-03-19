#pragma once

#include <functional>

#include <RendererConstants.h>
#include <RenderPass.h>
#include <Fence.h>

class Scene;

class Renderer
{
public:
    Renderer(HWND hWnd, RECT windowRect);
    ~Renderer();

    static ComPtr<ID3D12Device2> device() { return s_device; }

    void setCurrentScene( Scene const& scene ) { m_currentScene = &scene; }

    using ImguiCallback = std::function<void( void )>;
    void registerImguiCallback( ImguiCallback const& callback ) { m_imguiUserCallback = callback; m_imguiCallbackRegistered = true; }

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
    void recordImguiCommandList();

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

    ComPtr<ID3D12GraphicsCommandList> m_imguiCommandList;
    ComPtr<ID3D12CommandAllocator> m_imguiCommandAllocators[ RendererConstants::sc_numBackBuffers ];
    bool m_imguiCallbackRegistered;
    ImguiCallback m_imguiUserCallback;

    Scene const* m_currentScene;
};

