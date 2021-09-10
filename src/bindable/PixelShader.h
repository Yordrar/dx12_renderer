#pragma once

#include <string>

#include <bindable/IBindable.h>

class PixelShader : public IBindable
{
public:
	PixelShader( std::string filename );
	~PixelShader();

	virtual void bind( Renderer::RenderContext& context ) override;

private:
	ComPtr<ID3DBlob> m_bytecode;
};