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
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameters[ 2 ] = { {}, {} };

	// First is the camera CBV
	slotRootParameters[ 0 ].InitAsConstantBufferView( 0, 0 );
	CD3DX12_DESCRIPTOR_RANGE descRange;
	descRange.Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, static_cast<UINT>(m_shaderResources.size()), 0 );
	slotRootParameters[ 1 ].InitAsDescriptorTable( 1, &descRange );

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc( 2, slotRootParameters, 0, nullptr,
											 D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
											 D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
											 D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
											 D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
											 D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
											 D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS );

	// Create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature( &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
											  serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf() );
	Renderer::device()->CreateRootSignature( 0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS( m_rootSignature.GetAddressOf() ) );
	
	context.m_commandList->SetGraphicsRootSignature( m_rootSignature.Get() );

	// Descriptor heaps
	ID3D12DescriptorHeap* descriptorHeaps[] = { context.m_cameraDescriptors.Get(), m_descriptorHeap.Get() };
	context.m_commandList->SetDescriptorHeaps( _countof(descriptorHeaps), descriptorHeaps );
}
