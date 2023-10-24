#pragma once

#include <RendererConstants.h>
#include <Scene.h>
#include <ComputePass.h>
#include <resource/Resource.h>

class RenderPass
{
public:
    RenderPass( wchar_t const* name,
                wchar_t const* techniqueName,
                Descriptor renderTarget,
                Descriptor depthStencilTarget );
    ~RenderPass();

    ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.Get(); }
    double getExecutionTimeMilliseconds() const { return m_executionTimeInMilliseconds; }

    void record( Scene const& scene, std::vector<Camera*> const& cameras );
    void addResourceView( Descriptor const& descriptor );
    void addComputePassToWaitOn( ComputePass* computePass );
    bool hasToWaitOnCompute() const { return m_computePassesToWaitOn.size() > 0; }
    void waitOnComputePasses( ComPtr<ID3D12CommandQueue> cmdQueue, std::vector<ComputePass*> const& submittedComputePasses );

private:
    struct PassBufferData
    {
        uint32_t passResourceIndicesBufferIndex;
    };

    std::wstring m_name;
    std::wstring m_techniqueName;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[ RendererConstants::sc_numBackBuffers ];

    Descriptor m_renderTarget;
    Descriptor m_depthStencilTarget;

    D3D12_RECT m_scissorRect;

    uint64_t m_profilerQueryIndex;
    double m_executionTimeInMilliseconds;

    ResourceHandle m_passBuffer;
    PassBufferData m_passBufferData;

    ResourceHandle m_passResourceIndicesBuffer;
    std::vector<UINT> m_passResourceIndicesBufferData;

    std::vector<Descriptor> m_passResources;

    std::vector<ComputePass*> m_computePassesToWaitOn;
    std::vector<UINT> m_computePassFenceCounters;
};

