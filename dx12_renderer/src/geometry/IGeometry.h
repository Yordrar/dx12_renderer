#pragma once

#include <string>
#include <unordered_set>

#include <geometry/VertexBuffer.h>
#include <geometry/IndexBuffer.h>
#include <geometry/PSOManager.h>

class Resource;
class Descriptor;

class IGeometry
{
public:
    IGeometry( std::wstring name, std::initializer_list<std::wstring> techniqueNames );
    virtual ~IGeometry() = 0;

    virtual void record( std::wstring techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState );

    void addResourceView( Descriptor const& resourceView );

    std::wstring const& getName() const { return m_name; }
    std::unordered_set<std::wstring> const& getTechniqueNames() const { return m_techniqueNames; }

protected:
    std::wstring m_name;
    std::unordered_set<std::wstring> m_techniqueNames;
    Resource* m_bindlessIndices;
    std::vector<UINT> m_resourceIndices;
};