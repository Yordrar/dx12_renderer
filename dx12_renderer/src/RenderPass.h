#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>
#include <string>

#include <RendererConstants.h>
#include <Scene.h>
#include <resource/Texture.h>

class RenderPass
{
public:
    RenderPass( std::string name,
                std::string techniqueName,
                std::string renderTargetName,
                std::string depthStencilTargetName );
    ~RenderPass();

    ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.Get(); }

    void record( Scene& scene );

private:
    std::string m_name;
    std::string m_techniqueName;
    std::string m_renderTargetName;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[ RendererConstants::sc_numBackBuffers ];

    Texture* m_renderTarget;
    Texture* m_depthStencilTarget;
};

