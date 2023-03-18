#pragma once

#include <RendererConstants.h>
#include <Scene.h>
#include <resource/Resource.h>

class RenderPass
{
public:
    RenderPass( wchar_t const* name,
                wchar_t const* techniqueName,
                wchar_t const* renderTargetName,
                wchar_t const* depthStencilTargetName );
    ~RenderPass();

    ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.Get(); }
    double getExecutionTimeMilliseconds() const { return m_executionTimeInMilliseconds; }

    void record( Scene const& scene );

private:
    std::wstring m_name;
    std::wstring m_techniqueName;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[ RendererConstants::sc_numBackBuffers ];

    std::wstring m_renderTargetName;
    std::wstring m_depthStencilTargetName;
    Resource* m_renderTarget;
    Resource* m_depthStencilTarget;

    D3D12_RECT m_scissorRect;

    uint64_t m_profilerQueryIndex;
    double m_executionTimeInMilliseconds;
};

