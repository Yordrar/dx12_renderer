#include "Mesh.h"

#include <geometry/ShaderManager.h>
#include <resource/Resource.h>

Mesh::Mesh( wchar_t const* name, wchar_t const* materialName, D3D_PRIMITIVE_TOPOLOGY primitiveTopology )
    : m_name( name )
    , m_materialName( materialName )
    , m_vertexBuffer( nullptr )
    , m_indexBuffer( nullptr )
    , m_primitiveTopology( primitiveTopology )
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

bool Mesh::isAABBValid() const
{
    return DirectX::XMVector3LessOrEqual( m_aabb.m_minBounds, m_aabb.m_maxBounds );
}

void Mesh::record( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    std::vector<D3D12_RESOURCE_BARRIER> barriers;
    if ( m_vertexBuffer && m_vertexBuffer->getResource()->getResourceState() != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER )
    {
        D3D12_RESOURCE_BARRIER barrier = m_vertexBuffer->getResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER );
        barriers.push_back( barrier );
    }
    if ( m_indexBuffer && m_indexBuffer->getResource()->getResourceState() != D3D12_RESOURCE_STATE_INDEX_BUFFER )
    {
        D3D12_RESOURCE_BARRIER barrier = m_indexBuffer->getResource()->getTransitionBarrier( D3D12_RESOURCE_STATE_INDEX_BUFFER );
        barriers.push_back( barrier );
    }
    if ( barriers.size() > 0 )
    {
        commandList->ResourceBarrier( static_cast<UINT>( barriers.size() ), barriers.data() );
    }

    commandList->IASetPrimitiveTopology( m_primitiveTopology );
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
