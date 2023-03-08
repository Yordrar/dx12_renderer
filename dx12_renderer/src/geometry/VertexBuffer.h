#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class Resource;

class VertexBuffer
{
public:
	VertexBuffer( wchar_t const* name, void* vertices, size_t vertexSize, size_t vertexCount );
	virtual ~VertexBuffer();

	void bind( ComPtr<ID3D12GraphicsCommandList> commandList );

	size_t getVertexCount() const { return m_vertexCount; }

private:
	void* m_vertices;
	size_t m_vertexSize;
	size_t m_vertexCount;
	Resource* m_resource;
};