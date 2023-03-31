#pragma once

#include <d3d12.h>

class Resource;

class Descriptor
{
public:
	enum class Type
	{
		ShaderResourceView,
		ConstantBufferView,
		UnorderedAccessView,
		RenderTargetView,
		DepthStencilView,
	};

	Descriptor( Type type, 
				Resource* resource,
				D3D12_CPU_DESCRIPTOR_HANDLE const& cpuDescriptorHandleForHeapStart,
				UINT offsetInDescriptors,
				UINT descriptorIncrementSize,
				D3D12_DSV_FLAGS dsvFlags = D3D12_DSV_FLAG_NONE );
	~Descriptor();

	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_descriptor; }
	operator UINT() const { return m_descriptorIndex; }

	Resource* getResource() const { return m_resource; }
	Type getType() const { return m_type; }
	D3D12_CPU_DESCRIPTOR_HANDLE getView() const { return m_descriptor; }
	UINT getDescriptorIndex() const { return m_descriptorIndex; }
	D3D12_DSV_FLAGS getDSVFlags() const { return m_dsvFlags; }

private:
	Resource* m_resource;
	Type m_type;
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
	UINT m_descriptorIndex;
	D3D12_DSV_FLAGS m_dsvFlags;
};

