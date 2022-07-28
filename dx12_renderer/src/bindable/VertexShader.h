#pragma once

#include <string>

#include <d3dx12.h>
#include <dxcapi.h>

#include <bindable/IBindable.h>

class VertexShader : public IBindable
{
public:
	VertexShader( std::string filename );
	~VertexShader();

	virtual void bind( Renderer::RenderContext& context ) override;

private:
	ComPtr<IDxcBlob> m_bytecode;
};