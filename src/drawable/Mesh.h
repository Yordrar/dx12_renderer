#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include "IDrawable.h"

#include <resource/IBindable.h>
#include <resource/VertexBuffer.h>
#include <resource/IndexBuffer.h>
#include <resource/PixelShader.h>

template<typename vertex_t>
class Mesh : public IDrawable
{
public:
	Mesh(vertex_t* vertices, UINT vertexCount, UINT* indices, UINT indexCount);
	~Mesh();

private:
	VertexBuffer<vertex_t> m_vertexBuffer;
	IndexBuffer m_indexBuffer;
};