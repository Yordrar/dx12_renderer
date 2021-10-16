#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include "IDrawable.h"

#include <bindable/IBindable.h>
#include <bindable/VertexBuffer.h>
#include <bindable/IndexBuffer.h>

template<class vertex_t>
class Mesh : public IDrawable
{
public:
	Mesh(vertex_t* vertices, UINT vertexCount, UINT* indices, UINT indexCount);
	~Mesh();

private:
	VertexBuffer<vertex_t>* m_vertexBuffer;
	IndexBuffer* m_indexBuffer;
};

template<class vertex_t>
Mesh<vertex_t>::Mesh( vertex_t* vertices, UINT vertexCount, UINT* indices, UINT indexCount )
    : m_vertexBuffer( new VertexBuffer<vertex_t>( vertices, vertexCount ) )
    , m_indexBuffer( new IndexBuffer( indices, indexCount ) )
{
    addBindable( m_vertexBuffer );
    addBindable( m_indexBuffer );
}

template<class vertex_t>
Mesh<vertex_t>::~Mesh()
{

}