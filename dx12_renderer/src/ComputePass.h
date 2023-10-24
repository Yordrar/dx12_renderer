#pragma once

#include <RendererConstants.h>
#include <Scene.h>
#include <Fence.h>
#include <resource/Resource.h>

class ComputePass
{
public:
    ComputePass( wchar_t const* name,
                 wchar_t const* shaderFilename,
                 UINT threadGroupCountX,
                 UINT threadGroupCountY,
                 UINT threadGroupCountZ );
    ~ComputePass();

    ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.Get(); }
    double getExecutionTimeMilliseconds() const { return m_executionTimeInMilliseconds; }
    Fence& getFence() { return m_fence; }

    void record();
    void addResourceView( Descriptor const descriptor );
    void setResourceView( UINT index, Descriptor const descriptor );
    void setThreadGroupCountX( UINT threadGroupCountX ) { m_threadGroupCountX = threadGroupCountX; }
    void setThreadGroupCountY( UINT threadGroupCountY ) { m_threadGroupCountY = threadGroupCountY; }
    void setThreadGroupCountZ( UINT threadGroupCountZ ) { m_threadGroupCountZ = threadGroupCountZ; }
    void setThreadGroupCounts( UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ );
    void transitionResourcesForNextFrame( ComPtr<ID3D12GraphicsCommandList> commandList );

private:
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE m_rootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_CS m_computeShader;
    };
    struct PassBufferData
    {
        uint32_t passResourceIndicesBufferIndex;
    };

    std::wstring m_name;
    UINT m_threadGroupCountX;
    UINT m_threadGroupCountY;
    UINT m_threadGroupCountZ;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[ RendererConstants::sc_numBackBuffers ];

    ComPtr<ID3D12PipelineState> m_pso;

    uint64_t m_profilerQueryIndex;
    double m_executionTimeInMilliseconds;

    ResourceHandle m_passBuffer;
    PassBufferData m_passBufferData;

    ResourceHandle m_passResourceIndicesBuffer;
    std::vector<UINT> m_passResourceIndicesBufferData;

    std::vector<Descriptor> m_resourceViews;

    Fence m_fence;
};

