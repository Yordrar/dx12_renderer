#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>

#include <Renderer.h>
#include <resource/ResourceHandle.h>

class IResource
{
public:
	IResource() = default;
	virtual ~IResource() = 0;

	UINT getSlot();

	ComPtr<ID3D12Resource> getResource() const { return m_resource; }

	D3D12_GPU_VIRTUAL_ADDRESS getGPUVirtualAddress() const { return m_resource->GetGPUVirtualAddress(); }

	void updateResource( Renderer::RenderContext& context );

protected:
	void initInternalResources( CD3DX12_RESOURCE_DESC resourceDesc );
	UINT getSizeAligned256( UINT sizeInBytes );

	ResourceHandle m_handle;
	ComPtr<ID3D12Resource> m_resource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	std::vector<D3D12_SUBRESOURCE_DATA> m_subresourceData;
};