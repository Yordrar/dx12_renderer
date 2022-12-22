#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class VertexBuffer
{
public:
	VertexBuffer( void* vertices, UINT vertexSize, UINT vertexCount );
	virtual ~VertexBuffer();

	void bind( ComPtr<ID3D12GraphicsCommandList> commandList );

	UINT getVertexCount() const { return m_vertexCount; }

private:
	void* m_vertices;
	UINT m_vertexSize;
	UINT m_vertexCount;
	ComPtr<ID3D12Resource> m_bufferResource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	D3D12_SUBRESOURCE_DATA m_subResourceData;
	bool m_isDirty;
};