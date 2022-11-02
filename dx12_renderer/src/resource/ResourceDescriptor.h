#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>

#include <Renderer.h>

class ResourceDescriptor
{
public:
	ResourceDescriptor() = default;
	~ResourceDescriptor() = default;

protected:
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
	UINT m_descriptorIndex;
};