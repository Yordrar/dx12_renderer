#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <memory>

#include <Fence.h>

class Scene;

class Renderer
{
public:
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE m_rootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT m_inputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY m_topologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS m_vertexShader;
        CD3DX12_PIPELINE_STATE_STREAM_PS m_pixelShader;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT m_dsvFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS m_rtvFormats;
    };

    struct RenderContext
    {
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        PipelineStateStream m_pipelineState;
        ComPtr<ID3D12DescriptorHeap> m_cameraDescriptors;
    };

    Renderer(HWND hWnd, RECT windowRect);
    ~Renderer();

    static ComPtr<ID3D12Device2> device() { return m_device; }

    void renderScene(Scene& scene);

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
    ComPtr<ID3D12Resource> m_backBuffers[ sc_numBackBuffers ];
    ComPtr<ID3D12Resource> m_depthBuffers[ sc_numBackBuffers ];
    UINT m_currentBackBufferIndex;
    ComPtr<ID3D12RootSignature> m_rootSignature;
};

