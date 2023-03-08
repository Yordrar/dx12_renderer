#pragma once

#include <geometry/Material.h>
#include <geometry/PSOManager.h>
#include <geometry/VertexBuffer.h>
#include <geometry/IndexBuffer.h>

class Mesh
{
public:
    Mesh( wchar_t const* name, std::initializer_list<wchar_t const*> techniqueNames, Material& material );
    ~Mesh() = default;

    void setVertexBuffer( void* vertexData, UINT vertexSize, UINT vertexCount );
    void setIndexBuffer( UINT* indexData, UINT indexCount );

    std::vector<std::wstring> const& getTechniqueNames() const { return m_techniqueNames; }

    void record( wchar_t const* techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList );

private:
    std::wstring m_name;
    std::vector<std::wstring> m_techniqueNames;
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer> m_indexBuffer;

    Material m_material;
};