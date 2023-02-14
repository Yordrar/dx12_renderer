#include "Texture.h"

#include <d3dx12.h>

#include <resource/Resource.h>

Texture::Texture( std::wstring resourceName, std::string filename, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_TEXTURE_LAYOUT layout )
    : m_data( nullptr )
    , m_width( 0 )
    , m_height( 0 )
{
    int width = 0, height = 0;
    m_width = width;
    m_height = height;

    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( format, width, height, 1, 1, 1, 0, flags, layout );

    D3D12_SUBRESOURCE_DATA subresData;
    subresData.pData = m_data;
    subresData.RowPitch = width * 4;
    subresData.SlicePitch = 0;

    //m_resource = std::make_unique<Resource>( resourceName.c_str(), resourceDesc, subresData );

    //m_resource->setDebugName( resourceName );
}

Texture::Texture( std::wstring resourceName, UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_TEXTURE_LAYOUT layout )
    : m_data( nullptr )
    , m_width( width )
    , m_height( height )
{
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( format, width, height, 1, 1, 1, 0, flags, layout );

    D3D12_SUBRESOURCE_DATA subresData;
    subresData.pData = m_data;
    subresData.RowPitch = width * 4;
    subresData.SlicePitch = 0;

    //m_resource = std::make_unique<Resource>( resourceName, resourceDesc, subresData );

    //m_resource->setDebugName( resourceName );
}

Texture::Texture( std::wstring resourceName, ComPtr<ID3D12Resource> resource )
    : m_data( nullptr )
    , m_width( 0 )
    , m_height( 0 )
{
    //m_resource = std::make_unique<Resource>( resourceName, resource );

    //m_resource->setDebugName( resourceName );
}

Texture::~Texture()
{
    delete m_data;
}
