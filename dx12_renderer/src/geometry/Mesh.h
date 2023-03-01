#pragma once

#include <d3d12.h>

#include <vector>
#include <string>
#include <memory>
#include <list>

#include <geometry/Material.h>
#include <geometry/IGeometry.h>
#include <geometry/PSOManager.h>

class Mesh : public IGeometry
{
public:
    Mesh( wchar_t const* name, std::initializer_list<wchar_t const*> techniqueNames, Material& material );
    ~Mesh() = default;

    void setVertexBuffer( void* vertexData, UINT vertexSize, UINT vertexCount );
    void setIndexBuffer( UINT* indexData, UINT indexCount );

    void record( wchar_t const* techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList ) override;

private:
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer> m_indexBuffer;

    Material m_material;
};