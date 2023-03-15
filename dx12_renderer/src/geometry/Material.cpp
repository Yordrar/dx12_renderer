#include "Material.h"

#include <Renderer.h>
#include <geometry/PSOManager.h>
#include <geometry/ShaderManager.h>
#include <resource/ResourceManager.h>

Material::Material( MaterialDesc const& materialDesc )
    : m_desc( materialDesc )
{
    for ( Technique const& technique : m_desc.m_techniques )
    {
        ShaderManager::ShaderDesc vertexShaderDesc;
        vertexShaderDesc.m_filename = technique.m_shaderFilename;
        vertexShaderDesc.m_entryPoint = technique.m_name + L"_vs";
        vertexShaderDesc.m_shaderType = ShaderManager::ShaderType::VertexShader;
        vertexShaderDesc.m_enableDebug = true;

        ShaderManager::ShaderDesc pixelShaderDesc;
        pixelShaderDesc.m_filename = technique.m_shaderFilename;
        pixelShaderDesc.m_entryPoint = technique.m_name + L"_ps";
        pixelShaderDesc.m_shaderType = ShaderManager::ShaderType::PixelShader;
        pixelShaderDesc.m_enableDebug = true;

        PSOManager::PipelineStateStream pipelineStateStream;
        pipelineStateStream.m_rootSignature = Renderer::getRootSignature().Get();
        pipelineStateStream.m_vertexShader = ShaderManager::it().getShader( vertexShaderDesc );
        pipelineStateStream.m_pixelShader = ShaderManager::it().getShader( pixelShaderDesc );
        pipelineStateStream.m_blendState = technique.m_blendState;
        pipelineStateStream.m_rasterizerState = technique.m_rasterizerState;
        pipelineStateStream.m_depthStencilState = technique.m_depthStencilState;
        pipelineStateStream.m_inputLayout = D3D12_INPUT_LAYOUT_DESC{ m_desc.m_inputLayout.data(), static_cast<UINT>(m_desc.m_inputLayout.size()) };
        pipelineStateStream.m_topologyType = technique.m_topologyType;
        pipelineStateStream.m_rtFormats = technique.m_rtFormats;
        pipelineStateStream.m_dsFormat = technique.m_dsFormat;

        m_psoCache[ technique.m_name ] = PSOManager::it().getPSO( pipelineStateStream );
        m_psoCache[ technique.m_name ]->SetName( ( m_desc.m_name + L"/" + technique.m_name ).c_str() );
    }

    for ( Descriptor const& resourceView : m_desc.m_resourceViews )
    {
        m_bindlessIndices.push_back( resourceView.getDescriptorIndex() );
    }
    m_bindlessIndicesBufferResource = ResourceManager::it().createResource( ( m_desc.m_name + L"_bindlessIndicesBuffer" ).c_str(),
                                                                      CD3DX12_RESOURCE_DESC::Buffer( std::max( m_bindlessIndices.size() * sizeof( UINT ), 1Ui64 ) ),
                                                                      D3D12_SUBRESOURCE_DATA{ m_bindlessIndices.data(), static_cast<LONG_PTR>( m_bindlessIndices.size() * sizeof( UINT ) ), 0 } );

    m_materialBufferData.bindlessIndicesBufferIndex = m_bindlessIndicesBufferResource->getShaderResourceView()->getDescriptorIndex();
    m_materialBuffer = ResourceManager::it().createResource( ( m_desc.m_name + L"_materialBuffer" ).c_str(),
                                                             CD3DX12_RESOURCE_DESC::Buffer( std::max( sizeof( m_materialBufferData ), 1Ui64 ) ),
                                                             D3D12_SUBRESOURCE_DATA{ &m_materialBufferData, static_cast<LONG_PTR>( sizeof( m_materialBufferData ) ), 0 } );

}

ComPtr<ID3D12PipelineState> Material::getPSOForTechnique( wchar_t const* techniqueName ) const
{
    PSOCache::const_iterator it = m_psoCache.find( techniqueName );
    if ( it != m_psoCache.end() )
    {
        return it->second;
    }

    return nullptr;
}

bool Material::hasTechnique( wchar_t const* techniqueName ) const
{
    for ( Technique const& technique : m_desc.m_techniques )
    {
        if ( technique.m_name == techniqueName )
        {
            return true;
        }
    }
    return false;
}
