#include "ConstantBuffer.h"

#include <string>

#include <Renderer.h>
#include <resource/ResourceManager.h>

ConstantBuffer::ConstantBuffer( std::string name, void* data, UINT sizeInBytes )
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

    m_resource->SetName( std::wstring(name.begin(), name.end()).c_str() );
}

ConstantBuffer::~ConstantBuffer()
{

}
