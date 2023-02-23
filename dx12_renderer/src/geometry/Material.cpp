#include "Material.h"

#include <Renderer.h>
#include <geometry/PSOManager.h>
#include <geometry/ShaderManager.h>
#include <resource/ResourceManager.h>

Material::Material( MaterialDesc const& materialDesc )
    : m_desc( materialDesc )
{
    for ( std::wstring const& techniqueName : m_desc.m_techniqueNames )
    {
        ShaderManager::ShaderDesc vertexShaderDesc;
        vertexShaderDesc.m_filename = m_desc.m_vertexShaderFilename;
        vertexShaderDesc.m_entryPoint = techniqueName + L"_vs";
        vertexShaderDesc.m_shaderType = ShaderManager::ShaderType::VertexShader;
        vertexShaderDesc.m_enableDebug = true;

        ShaderManager::ShaderDesc pixelShaderDesc;
        pixelShaderDesc.m_filename = m_desc.m_pixelShaderFilename;
        pixelShaderDesc.m_entryPoint = techniqueName + L"_ps";
        pixelShaderDesc.m_shaderType = ShaderManager::ShaderType::PixelShader;
        pixelShaderDesc.m_enableDebug = true;

        PSOManager::PipelineStateStream pipelineStateStream;
        pipelineStateStream.m_rootSignature = Renderer::getRootSignature().Get();
        pipelineStateStream.m_vertexShader = ShaderManager::it().getShader( vertexShaderDesc );
        pipelineStateStream.m_pixelShader = ShaderManager::it().getShader( pixelShaderDesc );
        pipelineStateStream.m_blendState = m_desc.m_blendState;
        pipelineStateStream.m_rasterizerState = m_desc.m_rasterizerState;
        pipelineStateStream.m_depthStencilState = m_desc.m_depthStencilState;
        pipelineStateStream.m_inputLayout = D3D12_INPUT_LAYOUT_DESC{ m_desc.m_inputLayout.data(), static_cast<UINT>(m_desc.m_inputLayout.size()) };
        pipelineStateStream.m_topologyType = m_desc.m_topologyType;
        pipelineStateStream.m_rtFormats = m_desc.m_rtFormats;
        pipelineStateStream.m_dsFormat = m_desc.m_dsFormat;

        m_psoCache[ techniqueName ] = PSOManager::it().getPSO( pipelineStateStream );
        m_psoCache[ techniqueName ]->SetName( ( m_desc.m_name + L"/" + techniqueName ).c_str() );
    }

    for ( Descriptor const& resourceView : m_desc.m_resourceViews )
    {
        m_bindlessIndices.push_back( resourceView.getDescriptorIndex() );
    }
    m_bindlessIndicesResource = ResourceManager::it().createResource( ( m_desc.m_name + L"_bindlessBuffer" ).c_str(),
                                                                      CD3DX12_RESOURCE_DESC::Buffer( std::max( m_bindlessIndices.size() * sizeof( UINT ), 1Ui64 ) ),
                                                                      D3D12_SUBRESOURCE_DATA{ m_bindlessIndices.data(), static_cast<LONG_PTR>( m_bindlessIndices.size() * sizeof( UINT ) ), 0 } );
}

void Material::bindToPipeline( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    std::optional<CD3DX12_RESOURCE_BARRIER> barrier_optional = m_bindlessIndicesResource->getTransitionBarrier( D3D12_RESOURCE_STATE_GENERIC_READ );
    if ( barrier_optional.has_value() )
    {
        commandList->ResourceBarrier( 1, &barrier_optional.value() );
    }
    commandList->SetGraphicsRootConstantBufferView( 1, m_bindlessIndicesResource->getGPUVirtualAddress() );
}
