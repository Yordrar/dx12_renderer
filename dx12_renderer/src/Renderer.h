#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>

#include <RenderPass.h>
#include <Fence.h>

class Scene;

class Renderer
{
public:
    Renderer(HWND hWnd, RECT windowRect);
    ~Renderer();

    static ComPtr<ID3D12Device2> device() { return m_device; }

    void addRenderPass( RenderPass& renderPass );
    void drawScene( Scene& scene );

    static constexpr UINT sc_numBackBuffers = 3;
    static UINT getCurrentFrameIndex() { return s_currentBackBufferIndex; }
    static RECT getWindowRect() { return s_windowRect; }
    static ComPtr<ID3D12RootSignature> getRootSignature() { return s_rootSignature; }

private:
    HWND m_hWnd;
    static RECT s_windowRect;

    static ComPtr<ID3D12Device2> m_device;
    static UINT s_currentBackBufferIndex;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource> m_backBuffers[ sc_numBackBuffers ];
    ComPtr<ID3D12Resource> m_depthBuffers[ sc_numBackBuffers ];

    ComPtr<ID3D12CommandQueue> m_graphicsCmdQueue;
    std::unique_ptr<Fence> m_fence;
    ComPtr<ID3D12CommandQueue> m_computeCmdQueue;
    ComPtr<ID3D12CommandQueue> m_copyCmdQueue;

    std::vector<RenderPass> m_renderPasses;
    static ComPtr<ID3D12RootSignature> s_rootSignature;
};

