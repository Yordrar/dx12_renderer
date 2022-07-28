#include "Texture.h"

#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include <resource/stb_image.h>

#include <resource/ResourceManager.h>

Texture::Texture( std::string filename )
{
    int width, height, nrChannels;
    m_data = stbi_load( filename.c_str(), &width, &height, &nrChannels, 4 );
    m_width = width;
    m_height = height;

    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 6, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS );
    initInternalResources( resourceDesc );

    D3D12_SUBRESOURCE_DATA subresData;
    subresData.pData = m_data;
    subresData.RowPitch = width * 4;
    subresData.SlicePitch = 0;
    m_subresourceData.push_back( subresData );

    m_handle = ResourceManager::it()->getSrvCbvUavDescriptorHeap()->addSRV( m_resource, nullptr );
}

Texture::~Texture()
{
    delete m_data;
}
