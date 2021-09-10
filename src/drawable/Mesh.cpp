#include "Mesh.h"

template<typename vertex_t>
Mesh<vertex_t>::Mesh( vertex_t* vertices, UINT vertexCount, UINT* indices, UINT indexCount )
    : m_vertexBuffer(vertices, vertexCount)
    , m_indexBuffer(indices, indexCount)
{
    addBindable( &m_vertexBuffer );
    addBindable( &m_indexBuffer );
}

template<typename vertex_t>
Mesh<vertex_t>::~Mesh()
{

}