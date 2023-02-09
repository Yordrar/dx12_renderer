#pragma once

#include <d3d12.h>

class Descriptor
{
public:
	Descriptor( D3D12_CPU_DESCRIPTOR_HANDLE const& cpuDescriptorHandleForHeapStart, UINT offsetInDescriptors, UINT descriptorIncrementSize );
	~Descriptor();

	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_descriptor; }
	UINT getDescriptorIndex() const { return m_descriptorIndex; }

	D3D12_CPU_DESCRIPTOR_HANDLE getView() const { return m_descriptor; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
	UINT m_descriptorIndex;
};

