#include "TextureCube.h"

#include <stb_image.h>

TextureCube::TextureCube( std::string path )
{
	int width, height, nrChannels;
	m_textures.push_back(stbi_load((path + "\\px.png").c_str(), &width, &height, &nrChannels, 4));
	m_textures.push_back(stbi_load((path + "\\nx.png").c_str(), &width, &height, &nrChannels, 4));
	m_textures.push_back(stbi_load((path + "\\py.png").c_str(), &width, &height, &nrChannels, 4));
	m_textures.push_back(stbi_load((path + "\\ny.png").c_str(), &width, &height, &nrChannels, 4));
	m_textures.push_back(stbi_load((path + "\\pz.png").c_str(), &width, &height, &nrChannels, 4));
	m_textures.push_back(stbi_load((path + "\\nz.png").c_str(), &width, &height, &nrChannels, 4));
	
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 6, 6, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS );
	initInternalResources( resourceDesc );

	D3D12_SUBRESOURCE_DATA subresData;
	for ( int i = 0; i < 6; i++ )
	{
		subresData.pData = m_textures[ i ];
		subresData.RowPitch = width * 4;
		subresData.SlicePitch = 0;
		m_subresourceData.push_back( subresData );
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
	srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srv.TextureCube.MipLevels = 6;
	srv.TextureCube.MostDetailedMip = 0;
}

TextureCube::~TextureCube()
{
	for ( auto tex : m_textures )
	{
		delete tex;
	}
}
