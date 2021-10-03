#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <resource/ResourceHandle.h>

class ConstantBuffer
{
public:
	ConstantBuffer( UINT sizeInBytes );
	~ConstantBuffer();

	ComPtr<ID3D12Resource> getResource() const { return m_resource; }

	D3D12_GPU_VIRTUAL_ADDRESS getGPUBufferLocation() const { return m_resource->GetGPUVirtualAddress(); }

private:
	ComPtr<ID3D12Resource> m_resource;
	ResourceHandle m_handle;
	UINT m_sizeInBytes;
};
