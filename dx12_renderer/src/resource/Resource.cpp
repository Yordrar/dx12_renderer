#include "Resource.h"

#include <Renderer.h>
#include <resource/Descriptor.h>
#include <resource/DescriptorHeap.h>

Resource::Resource( wchar_t const* name, D3D12_RESOURCE_DESC const& resourceDesc, D3D12_SUBRESOURCE_DATA const& subresourceData )
    : m_name( name )
    , m_resource( nullptr )
    , m_intermediateUploadBuffer( nullptr )
    , m_subresourceData( subresourceData )
    , m_resourceState( D3D12_RESOURCE_STATE_COMMON )
    , m_needsCopyToGPU( subresourceData.pData != nullptr )
{
    FLOAT clearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    CD3DX12_CLEAR_VALUE clearValue;
    CD3DX12_CLEAR_VALUE* clearValuePtr = nullptr;

    if ( resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET )
    {
        clearValue = CD3DX12_CLEAR_VALUE( resourceDesc.Format, clearColor );
        clearValuePtr = &clearValue;
    }
    else if ( resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL )
    {
        clearValue = CD3DX12_CLEAR_VALUE( resourceDesc.Format, 1.0f, 0 );
        clearValuePtr = &clearValue;
    }

    CD3DX12_HEAP_PROPERTIES heapProperties( D3D12_HEAP_TYPE_DEFAULT );
    Renderer::device()->CreateCommittedResource( &heapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &resourceDesc,
                                                 D3D12_RESOURCE_STATE_COMMON,
                                                 clearValuePtr,
                                                 IID_PPV_ARGS( m_resource.GetAddressOf() ) );

    CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer( GetRequiredIntermediateSize( m_resource.Get(), 0, 1 ) );
    CD3DX12_HEAP_PROPERTIES uploadHeapProperties( D3D12_HEAP_TYPE_UPLOAD );
    Renderer::device()->CreateCommittedResource( &uploadHeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &uploadBufferResourceDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );
    setDebugName( name );
}

Resource::Resource( wchar_t const* name, ComPtr<ID3D12Resource> resource )
    : m_name( name )
    , m_resource( resource )
    , m_intermediateUploadBuffer( nullptr )
    , m_subresourceData( D3D12_SUBRESOURCE_DATA{ nullptr, 0, 0 } )
    , m_resourceState( D3D12_RESOURCE_STATE_COMMON )
    , m_needsCopyToGPU( false )
{

    CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
    CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer( GetRequiredIntermediateSize( m_resource.Get(), 0, 1 ) );
    Renderer::device()->CreateCommittedResource( &uploadHeapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &uploadBufferResourceDesc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );
    setDebugName( name );
}

Resource::~Resource()
{
}

Descriptor const* Resource::getConstantBufferView( D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc )
{
    cbvDesc.SizeInBytes = getSizeAligned256( cbvDesc.SizeInBytes );
    UINT descriptorIndex = DescriptorHeap::getDescriptorHeapCbvSrvUav().addCBV( &cbvDesc );
    return new Descriptor( Descriptor::Type::ConstantBufferView, 
                           this,
                           DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                           descriptorIndex,
                           DescriptorHeap::getDescriptorHeapCbvSrvUav().getIncrementSize() );
}

Descriptor const* Resource::getShaderResourceView( D3D12_SHADER_RESOURCE_VIEW_DESC const& srvDesc )
{
    UINT descriptorIndex = DescriptorHeap::getDescriptorHeapCbvSrvUav().addSRV( getD3DResource(), &srvDesc );
    return new Descriptor( Descriptor::Type::ShaderResourceView,
                           this,
                           DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                           descriptorIndex,
                           DescriptorHeap::getDescriptorHeapCbvSrvUav().getIncrementSize() );
}

Descriptor const* Resource::getUnorderedAccessView( D3D12_UNORDERED_ACCESS_VIEW_DESC const& uavDesc )
{
    UINT descriptorIndex = DescriptorHeap::getDescriptorHeapCbvSrvUav().addUAV( getD3DResource(), &uavDesc );
    return new Descriptor( Descriptor::Type::UnorderedAccessView,
                           this,
                           DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                           descriptorIndex,
                           DescriptorHeap::getDescriptorHeapCbvSrvUav().getIncrementSize() );
}

Descriptor const* Resource::getRenderTargetView( D3D12_RENDER_TARGET_VIEW_DESC const& rtvDesc )
{
    UINT descriptorIndex = DescriptorHeap::getDescriptorHeapRtv().addRTV( getD3DResource(), &rtvDesc );
    return new Descriptor( Descriptor::Type::RenderTargetView,
                           this,
                           DescriptorHeap::getDescriptorHeapRtv().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                           descriptorIndex,
                           DescriptorHeap::getDescriptorHeapRtv().getIncrementSize() );
}

Descriptor const* Resource::getDepthStencilView( D3D12_DEPTH_STENCIL_VIEW_DESC const& dsvDesc )
{
    UINT descriptorIndex = DescriptorHeap::getDescriptorHeapDsv().addDSV( getD3DResource(), &dsvDesc );
    return new Descriptor( Descriptor::Type::DepthStencilView,
                           this,
                           DescriptorHeap::getDescriptorHeapDsv().getHeap()->GetCPUDescriptorHandleForHeapStart(),
                           descriptorIndex,
                           DescriptorHeap::getDescriptorHeapDsv().getIncrementSize(),
                           dsvDesc.Flags );
}

CD3DX12_RESOURCE_BARRIER Resource::getTransitionBarrier( D3D12_RESOURCE_STATES newState )
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition
    (
        m_resource.Get(),
        m_resourceState,
        newState
    );

    m_resourceState = newState;

    return barrier;
}

void Resource::copyDataToGPU( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    assert( m_resourceState == D3D12_RESOURCE_STATE_COPY_DEST );
    assert( m_subresourceData.pData );
    UpdateSubresources<1>( commandList.Get(), m_resource.Get(), m_intermediateUploadBuffer.Get(), 0, 0, 1, &m_subresourceData );
    m_needsCopyToGPU = false;
}

void Resource::setDebugName( wchar_t const* debugName )
{
    m_resource->SetName( debugName );
}