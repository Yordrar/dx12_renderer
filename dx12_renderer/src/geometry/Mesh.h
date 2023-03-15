#pragma once

#include <geometry/Material.h>
#include <geometry/PSOManager.h>
#include <geometry/VertexBuffer.h>
#include <geometry/IndexBuffer.h>

class Mesh
{
public:
    Mesh( wchar_t const* name, wchar_t const* materialName );
    ~Mesh() = default;

    void setVertexBuffer( void* vertexData, UINT vertexSize, UINT vertexCount );
    void setIndexBuffer( UINT* indexData, UINT indexCount );

    std::wstring const& getMaterialName() const { return m_materialName; }

    void record( wchar_t const* techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList );

private:
    std::wstring m_name;
    std::wstring m_materialName;
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer> m_indexBuffer;
};