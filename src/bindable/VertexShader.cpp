#include "VertexShader.h"

#include <d3dx12.h>
#include <d3dcompiler.h>

#include <Renderer.h>

VertexShader::VertexShader( std::string filename )
{
	std::wstring stemp = std::wstring( filename.begin(), filename.end() );
	LPCWSTR sw = stemp.c_str();
	D3DReadFileToBlob( sw, m_bytecode.GetAddressOf() );
}

VertexShader::~VertexShader()
{

}

void VertexShader::bind( Renderer::RenderContext& context )
{
	context.m_pipelineState.VS = CD3DX12_SHADER_BYTECODE( m_bytecode.Get() );
}
