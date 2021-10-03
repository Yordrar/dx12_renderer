#include "PixelShader.h"

#include <d3dx12.h>
#include <d3dcompiler.h>

#include <Renderer.h>

PixelShader::PixelShader( std::string filename )
{
	std::wstring stemp = std::wstring( filename.begin(), filename.end() );
	LPCWSTR sw = stemp.c_str();
	D3DReadFileToBlob( sw, m_bytecode.GetAddressOf() );
}

PixelShader::~PixelShader()
{

}

void PixelShader::bind( Renderer::RenderContext& context )
{
	context.m_pipelineState.m_pixelShader = CD3DX12_SHADER_BYTECODE( m_bytecode.Get() );
}
