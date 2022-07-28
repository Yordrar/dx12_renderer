#include "Material.h"

#include <d3d12.h>
#include <d3dx12.h>

Material::Material( std::string vertexShaderFilename, std::string pixelShaderFilename )
	: m_vertexShader(vertexShaderFilename)
	, m_pixelShader(pixelShaderFilename)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 10;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	Renderer::device()->CreateDescriptorHeap( &heapDesc, IID_PPV_ARGS( m_descriptorHeap.GetAddressOf() ) );
}

Material::~Material()
{
}

void Material::bind( Renderer::RenderContext& context )
{

}
