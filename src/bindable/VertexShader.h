#pragma once

#include <string>

#include <bindable/IBindable.h>

class VertexShader : public IBindable
{
public:
	VertexShader( std::string filename );
	~VertexShader();

	virtual void bind( Renderer::RenderContext& context ) override;

private:
	ComPtr<ID3DBlob> m_bytecode;
};