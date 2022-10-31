#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>

#include <Renderer.h>

class IResource
{
	friend class ResourceManager;
public:
	IResource() = default;
	virtual ~IResource() = 0;

	UINT getDescriptorHeapIndex() const { return m_descriptorIndex; }
	D3D12_CPU_DESCRIPTOR_HANDLE getDescriptor() const { return m_descriptor; }

	ComPtr<ID3D12Resource> getResource() const { return m_resource; }
	D3D12_GPU_VIRTUAL_ADDRESS getGPUVirtualAddress() const { return m_resource->GetGPUVirtualAddress(); }

	void transitionToState( ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState );

protected:
	void initInternalResources( CD3DX12_RESOURCE_DESC resourceDesc );
	UINT getSizeAligned256( UINT sizeInBytes );

	ComPtr<ID3D12Resource> m_resource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	std::vector<D3D12_SUBRESOURCE_DATA> m_subresourceData;

	UINT m_descriptorIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;

	D3D12_RESOURCE_STATES m_resourceState;
};