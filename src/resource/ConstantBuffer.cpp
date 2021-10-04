#include "ConstantBuffer.h"

#include <Renderer.h>
#include <resource/ResourceManager.h>

ConstantBuffer::ConstantBuffer( UINT sizeInBytes )
    : m_handle(0)
    , m_sizeInBytes(sizeInBytes)
{
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( sizeInBytes );
    Renderer::device()->CreateCommittedResource( &heapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &resourceDesc,
                                                 D3D12_RESOURCE_STATE_COMMON,
                                                 nullptr,
                                                 IID_PPV_ARGS( m_resource.GetAddressOf() ) );

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
    cbv.BufferLocation = m_resource->GetGPUVirtualAddress();
    cbv.SizeInBytes = sizeInBytes;

    m_handle = ResourceManager::it()->getSrvCbvUavDescriptorHeap()->addCBV( &cbv );
}

ConstantBuffer::~ConstantBuffer()
{

}
