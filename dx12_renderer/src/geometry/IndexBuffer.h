#pragma once

class Resource;

class IndexBuffer
{
public:
	IndexBuffer( wchar_t const* name, UINT* indices, UINT count );
	virtual ~IndexBuffer();

	void bind( ComPtr<ID3D12GraphicsCommandList> commandList );

	UINT getIndexCount() const { return m_indexCount; }
	Resource* getResource() const { return m_resource; }

private:
	UINT* m_indices;
	UINT m_indexCount;
	Resource* m_resource;
};
