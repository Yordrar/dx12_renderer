#pragma once

#include <vector>

#include <bindable/IBindable.h>

class IndexBuffer : public IBindable
{
public:
	IndexBuffer( UINT* indices, UINT count );
	virtual ~IndexBuffer();

	virtual void bind( Renderer::RenderContext& context ) override;

	UINT getIndexCount() const { return m_indexCount; }

private:
	UINT* m_indices;
	UINT m_indexCount;
	ComPtr<ID3D12Resource> m_bufferResource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	D3D12_SUBRESOURCE_DATA m_subResourceData;
	bool m_isDirty;
};
