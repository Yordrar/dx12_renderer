#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>

class Resource;

class Texture
{
public:
	Texture( std::wstring resourceName, std::string filename, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN );
	Texture( std::wstring resourceName, UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN );
	Texture( std::wstring resourceName, ComPtr<ID3D12Resource> resource );
	~Texture();

	Resource const& getResource() const { return *m_resource; }

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	DXGI_FORMAT getFormat() const { return m_resource->GetDesc().Format; }

private:
	std::unique_ptr<Resource> m_resource;
	unsigned char* m_data;
	int m_width;
	int m_height;
};