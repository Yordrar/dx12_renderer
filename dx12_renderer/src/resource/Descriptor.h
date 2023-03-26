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
		UniformAccessView,
		RenderTargetView,
		DepthStencilView,
	};

	Descriptor( Resource* resource,
				Type type,
				D3D12_CPU_DESCRIPTOR_HANDLE const& cpuDescriptorHandleForHeapStart,
				UINT offsetInDescriptors,
				UINT descriptorIncrementSize );
	~Descriptor();

	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_descriptor; }
	operator UINT() const { return m_descriptorIndex; }

	Resource* getResource() const { return m_resource; }
	Type getType() const { return m_type; }
	D3D12_CPU_DESCRIPTOR_HANDLE getView() const { return m_descriptor; }
	UINT getDescriptorIndex() const { return m_descriptorIndex; }

private:
	Resource* m_resource;
	Type m_type;
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
	UINT m_descriptorIndex;
};

