#pragma once

#include <string>
#include <unordered_set>

#include <geometry/VertexBuffer.h>
#include <geometry/IndexBuffer.h>
#include <geometry/PSOManager.h>

class IGeometry
{
public:
    IGeometry( std::string name, std::initializer_list<std::string> renderPassNames );
    virtual ~IGeometry() = 0;

    virtual void recordRenderPass( std::string techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState ) = 0;

    std::string getName() const { return m_name; }
    std::unordered_set<std::string> const& getRenderPassNames() const { return m_renderPassNames; }

protected:
    std::string m_name;
    std::unordered_set<std::string> m_renderPassNames;
};