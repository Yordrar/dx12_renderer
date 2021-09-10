#pragma once

#include <vector>

#include <d3dx12.h>

#include <bindable/IBindable.h>

template<typename vertex_t>
class VertexBuffer : public IBindable
{
public:
	VertexBuffer( vertex_t* vertices, UINT count );
	virtual ~VertexBuffer();

	virtual void bind( Renderer::RenderContext& context ) override;

private:
	vertex_t* m_vertices;
	UINT m_vertexCount;
	ComPtr<ID3D12Resource> m_bufferResource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	D3D12_SUBRESOURCE_DATA m_subResourceData;
	bool m_isDirty;
};
