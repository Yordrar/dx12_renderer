#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>

#include <resource/IResource.h>

class Texture : public IResource
{
public:
	Texture( std::string resourceName, std::string filename, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN );
	Texture( std::string resourceName, UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN );
	Texture( std::string resourceName, ComPtr<ID3D12Resource> resource );
	~Texture();

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	DXGI_FORMAT getFormat() const { return m_resource->GetDesc().Format; }

private:
	unsigned char* m_data;
	int m_width;
	int m_height;
};