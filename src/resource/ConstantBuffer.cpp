#include "ConstantBuffer.h"

#include <Renderer.h>
#include <resource/ResourceManager.h>

ConstantBuffer::ConstantBuffer(void* data, UINT sizeInBytes, LPCWSTR debugName )
    : m_data(data)
    , m_sizeInBytes( sizeInBytes )
    , m_alignedSizeInBytes( getSizeAligned256( sizeInBytes ) )
{
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( m_alignedSizeInBytes );
    initInternalResources( resourceDesc );

    D3D12_SUBRESOURCE_DATA subresData;
    subresData.pData = data;
    subresData.RowPitch = m_alignedSizeInBytes;
    subresData.SlicePitch = 0;
    m_subresourceData.push_back( subresData );

    if ( debugName )
    {
        m_resource->SetName( debugName );
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
    cbv.BufferLocation = getGPUVirtualAddress();
    cbv.SizeInBytes = m_alignedSizeInBytes;

    m_handle = ResourceManager::it()->getSrvCbvUavDescriptorHeap()->addCBV( &cbv );
}

ConstantBuffer::~ConstantBuffer()
{

}
