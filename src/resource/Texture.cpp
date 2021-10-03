#include "Texture.h"

#include <resource/ResourceManager.h>

Texture::Texture( std::string filename )
    : m_handle(0)
{
    CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1 );
    Renderer::device()->CreateCommittedResource( &heapProperties,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &resourceDesc,
                                                 D3D12_RESOURCE_STATE_COPY_DEST,
                                                 nullptr,
                                                 IID_PPV_ARGS(m_resource.GetAddressOf()) );

    m_handle = ResourceManager::it()->getSrvCbvUavDescriptorHeap()->addSRV( m_resource, nullptr );
}

Texture::~Texture()
{

}

void Texture::bind( Renderer::RenderContext& context )
{

}
