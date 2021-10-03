#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>

#include <resource/IBindable.h>
#include <resource/ResourceHandle.h>

class Texture : public IBindable
{
public:
	Texture(std::string filename);
	~Texture();

	virtual void bind( Renderer::RenderContext& context ) override;

private:
	ComPtr<ID3D12Resource> m_resource;
	ResourceHandle m_handle;
};