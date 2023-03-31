#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <resource/Descriptor.h>

class Resource
{
	friend class ResourceManager;
public:
	Resource( wchar_t const* name, D3D12_RESOURCE_DESC const& resourceDesc, D3D12_SUBRESOURCE_DATA const& subresourceData = D3D12_SUBRESOURCE_DATA{ nullptr, 0, 0 } );
	Resource( wchar_t const* name, ComPtr<ID3D12Resource> resource );
	~Resource();

	Resource( const Resource& ) = delete;
	Resource& operator= ( const Resource& ) = delete;

	static UINT getSizeAligned256( UINT sizeInBytes ) { return ( sizeInBytes + 255 ) & ~255; }

	std::wstring const& getName() const { return m_name; }
	ComPtr<ID3D12Resource> getD3DResource() const { return m_resource; }
	D3D12_RESOURCE_DESC getResourceDesc() const { return m_resource->GetDesc(); }
	D3D12_GPU_VIRTUAL_ADDRESS getGPUVirtualAddress() const { return m_resource->GetGPUVirtualAddress(); }
	void setNeedsCopyToGPU( bool needsCopy ) { m_needsCopyToGPU = needsCopy; }
	bool getNeedsCopyToGPU() const { return m_needsCopyToGPU; }
	Descriptor const* getConstantBufferView( D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc );
	Descriptor const* getShaderResourceView( D3D12_SHADER_RESOURCE_VIEW_DESC const& srvDesc );
	Descriptor const* getUnorderedAccessView( D3D12_UNORDERED_ACCESS_VIEW_DESC const& uavDesc );
	Descriptor const* getRenderTargetView( D3D12_RENDER_TARGET_VIEW_DESC const& rtvDesc );
	Descriptor const* getDepthStencilView( D3D12_DEPTH_STENCIL_VIEW_DESC const& dsvDesc );
	D3D12_RESOURCE_STATES getResourceState() const { return m_resourceState; }

	CD3DX12_RESOURCE_BARRIER getTransitionBarrier( D3D12_RESOURCE_STATES newState );
	void copyDataToGPU( ComPtr<ID3D12GraphicsCommandList> commandList );
	void setDebugName( wchar_t const* debugName );

private:
	std::wstring m_name;

	ComPtr<ID3D12Resource> m_resource;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;
	D3D12_SUBRESOURCE_DATA m_subresourceData;

	D3D12_RESOURCE_STATES m_resourceState;

	bool m_needsCopyToGPU;
};