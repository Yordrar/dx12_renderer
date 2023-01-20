#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <optional>

class Descriptor;

class Resource
{
	friend class ResourceManager;
public:
	Resource( D3D12_RESOURCE_DESC& resourceDesc );
	Resource( D3D12_RESOURCE_DESC& resourceDesc, D3D12_SUBRESOURCE_DATA& subresourceData );
	Resource( ComPtr<ID3D12Resource> resource );
	~Resource();

	ComPtr<ID3D12Resource> getResource() const { return m_resource; }
	D3D12_RESOURCE_DESC getResourceDesc() const {return m_resource->GetDesc(); }
	D3D12_GPU_VIRTUAL_ADDRESS getGPUVirtualAddress() const { return m_resource->GetGPUVirtualAddress(); }

	std::optional<CD3DX12_RESOURCE_BARRIER> getTransitionBarrier( ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState );
	void copyDataToGPU( ComPtr<ID3D12GraphicsCommandList> commandList );
	void setDebugName( std::wstring& debugName );

protected:
	UINT getSizeAligned256( UINT sizeInBytes );

	ComPtr<ID3D12Resource> m_resource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	D3D12_SUBRESOURCE_DATA m_subresourceData;

	std::unique_ptr<Descriptor> m_srv;
	std::unique_ptr<Descriptor> m_cbv;
	std::unique_ptr<Descriptor> m_uav;
	std::unique_ptr<Descriptor> m_rtv;
	std::unique_ptr<Descriptor> m_dsv;

	D3D12_RESOURCE_STATES m_resourceState;

	bool m_needsCopyToGPU;
};