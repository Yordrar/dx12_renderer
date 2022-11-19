#pragma once

#include <string>
#include <unordered_set>

#include <geometry/VertexBuffer.h>
#include <geometry/IndexBuffer.h>
#include <geometry/PSOManager.h>

class IResource;

class IGeometry
{
public:
    IGeometry( std::string name, std::initializer_list<std::string> techniqueNames );
    virtual ~IGeometry() = 0;

    virtual void record( std::string techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState );

    void addResource( IResource* resource );

    std::string getName() const { return m_name; }
    std::unordered_set<std::string> const& getTechniqueNames() const { return m_techniqueNames; }

protected:
    std::string m_name;
    std::unordered_set<std::string> m_techniqueNames;
    std::vector< IResource* > m_resources;
};