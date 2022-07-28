#pragma once

#include <string>

#include <d3dx12.h>
#include <dxcapi.h>

#include <bindable/IBindable.h>

class PixelShader : public IBindable
{
public:
	PixelShader( std::string filename );
	~PixelShader();

	virtual void bind( Renderer::RenderContext& context ) override;

private:
	ComPtr<IDxcBlob> m_bytecode;
};