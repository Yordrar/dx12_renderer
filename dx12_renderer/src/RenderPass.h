#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>
#include <string>

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

    void addScene( Scene* scene ) { m_scenes.push_back( scene ); }
    void record();

private:
    std::wstring m_name;
    std::wstring m_techniqueName;
    std::wstring m_renderTargetName;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[ RendererConstants::sc_numBackBuffers ];

    Resource* m_renderTarget;
    Resource* m_depthStencilTarget;

    std::vector<Scene*> m_scenes;

    uint64_t m_profilerQueryIndex;
    double m_executionTimeInMilliseconds;
};

