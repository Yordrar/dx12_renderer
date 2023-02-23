#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class VertexBuffer
{
public:
	VertexBuffer( void* vertices, size_t vertexSize, size_t vertexCount );
	virtual ~VertexBuffer();

	void bind( ComPtr<ID3D12GraphicsCommandList> commandList );

	size_t getVertexCount() const { return m_vertexCount; }

private:
	void* m_vertices;
	size_t m_vertexSize;
	size_t m_vertexCount;
	ComPtr<ID3D12Resource> m_bufferResource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	D3D12_SUBRESOURCE_DATA m_subResourceData;
	bool m_isDirty;
};