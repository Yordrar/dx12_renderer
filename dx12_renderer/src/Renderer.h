#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>
#include <thread>

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

    void addRenderPass( RenderPass& renderPass );
    void drawScene( Scene& scene );

    void presentThreadFunc();

    static UINT getCurrentRecordingIndex() { return s_currentRecordingIndex; }
    static UINT getPreviousRecordingIndex() { return std::abs(static_cast<int>(s_currentRecordingIndex) - 1); }
    static RECT getWindowRect() { return s_windowRect; }
    static ComPtr<ID3D12RootSignature> getRootSignature() { return s_rootSignature; }

private:
    HWND m_hWnd;
    static RECT s_windowRect;

    static ComPtr<ID3D12Device2> s_device;
    static UINT s_currentBackBufferIndex;
    static UINT s_currentRecordingIndex;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource> m_backBuffers[ RendererConstants::sc_numBackBuffers ];
    std::unique_ptr<Fence> m_frameFences[ RendererConstants::sc_numBackBuffers ];

    ComPtr<ID3D12CommandQueue> m_graphicsCmdQueue;
    ComPtr<ID3D12CommandQueue> m_computeCmdQueue;
    ComPtr<ID3D12CommandQueue> m_copyCmdQueue;

    std::vector<RenderPass> m_renderPasses;
    static ComPtr<ID3D12RootSignature> s_rootSignature;

    std::unique_ptr<std::thread> m_presentThread;
    bool m_terminatePresentThread;
};

