#include "Mesh.h"

#include <geometry/ShaderManager.h>
#include <geometry/PSOManager.h>
#include <resource/Resource.h>

Mesh::Mesh( wchar_t const* name, wchar_t const* materialName )
    : m_name( name )
    , m_materialName( materialName )
    , m_vertexBuffer( nullptr )
    , m_indexBuffer( nullptr )
{
}

void Mesh::setVertexBuffer( void* vertexData, UINT vertexSize, UINT vertexCount )
{
    m_vertexBuffer = std::make_unique<VertexBuffer>( ( m_name + L"_vertexbuffer" ).c_str(), vertexData, vertexSize, vertexCount);
}

void Mesh::setIndexBuffer( UINT* indexData, UINT indexCount )
{
    m_indexBuffer = std::make_unique<IndexBuffer>( ( m_name + L"_indexbuffer" ).c_str(), indexData, indexCount );
}

void Mesh::record( wchar_t const* techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList )
{
    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_vertexBuffer->bind( commandList );

    if( m_indexBuffer )
    {
        m_indexBuffer->bind(commandList);
        commandList->DrawIndexedInstanced(m_indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }
    else
    {
        commandList->DrawInstanced(static_cast<UINT>( m_vertexBuffer->getVertexCount() ), 1, 0, 0);
    }
}
