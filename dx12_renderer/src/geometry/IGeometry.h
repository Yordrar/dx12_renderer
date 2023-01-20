#pragma once

#include <string>
#include <unordered_set>

#include <geometry/VertexBuffer.h>
#include <geometry/IndexBuffer.h>
#include <geometry/PSOManager.h>
#include <resource/ConstantBuffer.h>

class Resource;

class IGeometry
{
public:
    IGeometry( std::wstring name, std::initializer_list<std::wstring> techniqueNames );
    virtual ~IGeometry() = 0;

    virtual void record( std::wstring techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState );

    void addResource( Resource* resource );

    std::wstring getName() const { return m_name; }
    std::unordered_set<std::wstring> const& getTechniqueNames() const { return m_techniqueNames; }

protected:
    std::wstring m_name;
    std::unordered_set<std::wstring> m_techniqueNames;
    std::vector< Resource* > m_resources;
    ConstantBuffer* m_bindlessIndices;
    std::vector<float> m_resourceIndices;
    bool m_isDirty;
};