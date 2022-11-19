#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture( std::string resourceName, std::string filename, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_TEXTURE_LAYOUT layout )
    : m_data( nullptr )
    , m_width( 0 )
    , m_height( 0 )
{
    int width, height, nrChannels;
    m_data = stbi_load( filename.c_str(), &width, &height, &nrChannels, 4 );
    m_width = width;
    m_height = height;

    D3D12_SUBRESOURCE_DATA subresData;
    subresData.pData = m_data;
    subresData.RowPitch = width * nrChannels;
    subresData.SlicePitch = 0;
    m_subresourceData.push_back( subresData );

    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( format, width, height, 1, 6, 1, 0, flags, layout );
    initInternalResources( resourceDesc );

    m_needsCopyToGPU = true;

    m_resource->SetName( std::wstring( resourceName.begin(), resourceName.end() ).c_str() );
}

Texture::Texture( std::string resourceName, UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_TEXTURE_LAYOUT layout )
    : m_data( nullptr )
    , m_width( width )
    , m_height( height )
{
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( format, width, height, 1, 6, 1, 0, flags, layout );
    initInternalResources( resourceDesc );

    m_resource->SetName( std::wstring( resourceName.begin(), resourceName.end() ).c_str() );
}

Texture::Texture( std::string resourceName, ComPtr<ID3D12Resource> resource )
    : m_data( nullptr )
    , m_width( 0 )
    , m_height( 0 )
{
    m_resource = resource;
    m_resource->SetName( std::wstring( resourceName.begin(), resourceName.end() ).c_str() );
}

Texture::~Texture()
{
    delete m_data;
}
