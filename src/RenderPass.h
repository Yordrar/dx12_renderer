#pragma once

#include <RendererConstants.h>
#include <Scene.h>
#include <ComputePass.h>
#include <resource/Resource.h>

class Renderer;

class RenderPass
{
public:
    RenderPass( wchar_t const* name,
                wchar_t const* techniqueName,
                Descriptor renderTarget,
                Descriptor depthStencilTarget );
    ~RenderPass();

    double getExecutionTimeMilliseconds() const { return m_executionTimeInMilliseconds; }

    void record( Renderer& renderer, ComPtr<ID3D12GraphicsCommandList> commandList, Scene& scene, std::vector<Camera*> const& cameras );

    void addResourceView( Descriptor const& descriptor );
    void setScissorRect( D3D12_RECT const& rect );
    
    void setDepthEnable(bool enable);
    void setDepthWriteMask(D3D12_DEPTH_WRITE_MASK depthWriteMask);
    void setDepthFunc(D3D12_COMPARISON_FUNC depthFunc);

    void setBlendDesc(D3D12_BLEND_DESC const& blendDesc) { m_blendState = CD3DX12_BLEND_DESC(blendDesc); }
    
    void setClearRenderTargetsBeforeRendering(bool clearTargets) { m_clearRenderTargetsBeforeRendering = clearTargets; }

    void addFenceToSignal( Fence& fence );
    std::vector<Fence*> const& getFencesToSignal() const { return m_fencesToSignal; }
    void addFenceToWaitOn( Fence& fence );
    std::vector<Fence*> const& getFencesToWaitOn() const { return m_fencesToWaitOn; }

private:
    std::wstring m_name;
    std::wstring m_techniqueName;

    Descriptor m_renderTarget;
    Descriptor m_depthStencilTarget;
    bool m_clearRenderTargetsBeforeRendering;
    CD3DX12_DEPTH_STENCIL_DESC m_depthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT{});

    CD3DX12_BLEND_DESC m_blendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT{});

    CD3DX12_RT_FORMAT_ARRAY m_rtFormats;
    DXGI_FORMAT m_dsFormat;

    D3D12_RECT m_scissorRect;
    bool m_useCustomScissorRect;

    int64_t m_profilerQueryIndex;
    double m_executionTimeInMilliseconds;

    ResourceHandle m_passBuffer;
    std::vector<UINT> m_passBufferData;
    Descriptor m_passBufferDescriptor;

    std::vector<Descriptor> m_passResources;

    std::vector<Fence*> m_fencesToSignal;
    std::vector<Fence*> m_fencesToWaitOn;
};

