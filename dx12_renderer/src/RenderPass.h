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
    void addComputePassToWaitOn( ComputePass* computePass );
    bool hasToWaitOnCompute() const { return m_computePassesToWaitOn.size() > 0; }
    void waitOnComputePasses( ComPtr<ID3D12CommandQueue> cmdQueue, std::vector<ComputePass*> const& submittedComputePasses );
    void setScissorRect( D3D12_RECT const& rect );

private:
    std::wstring m_name;
    std::wstring m_techniqueName;

    Descriptor m_renderTarget;
    Descriptor m_depthStencilTarget;

    D3D12_RECT m_scissorRect;
    bool m_useCustomScissorRect;

    int64_t m_profilerQueryIndex;
    double m_executionTimeInMilliseconds;

    ResourceHandle m_passBuffer;
    std::vector<UINT> m_passBufferData;
    Descriptor m_passBufferDescriptor;

    std::vector<Descriptor> m_passResources;

    std::vector<ComputePass*> m_computePassesToWaitOn;
    std::vector<UINT> m_computePassFenceCounters;
};

