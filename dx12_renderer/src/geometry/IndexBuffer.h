#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>

class Resource;

class IndexBuffer
{
public:
	IndexBuffer( wchar_t const* name, UINT* indices, UINT count );
	virtual ~IndexBuffer();

	void bind( ComPtr<ID3D12GraphicsCommandList> commandList );

	UINT getIndexCount() const { return m_indexCount; }

private:
	UINT* m_indices;
	UINT m_indexCount;
	Resource* m_resource;
};
