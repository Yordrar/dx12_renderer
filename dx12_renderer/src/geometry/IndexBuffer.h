#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>

class IndexBuffer
{
public:
	IndexBuffer( UINT* indices, UINT count );
	virtual ~IndexBuffer();

	void bind( ComPtr<ID3D12GraphicsCommandList> commandList );

	UINT getIndexCount() const { return m_indexCount; }

private:
	UINT* m_indices;
	UINT m_indexCount;
	ComPtr<ID3D12Resource> m_bufferResource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	D3D12_SUBRESOURCE_DATA m_subResourceData;
	bool m_isDirty;
};
