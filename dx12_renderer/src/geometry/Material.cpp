#include "Material.h"

#include <Renderer.h>
#include <geometry/ShaderManager.h>
#include <geometry/Mesh.h>
#include <resource/ResourceManager.h>

Material::Material( MaterialDesc const& materialDesc )
    : m_desc( materialDesc )
{
    for ( Descriptor const& resourceView : m_desc.m_resourceViews )
    {
        m_bindlessIndices.push_back( resourceView.getDescriptorIndex() );
    }
    m_materialBuffer = ResourceManager::it().createResource( ( m_desc.m_name + L"_bindlessIndicesBuffer" ).c_str(),
                                                                      CD3DX12_RESOURCE_DESC::Buffer( std::max( m_bindlessIndices.size() * sizeof( UINT ), 1Ui64 ) ),
                                                                      D3D12_SUBRESOURCE_DATA{ m_bindlessIndices.data(), static_cast<LONG_PTR>( m_bindlessIndices.size() * sizeof( UINT ) ), 0 } );
    
    D3D12_RESOURCE_DESC materialBufferResourceDesc = ResourceManager::it().getResourceDesc(m_materialBuffer);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
    {
        .Format = DXGI_FORMAT_R32_TYPELESS,
        .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    };
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
    srvDesc.Buffer.NumElements = static_cast<UINT>(std::max(m_bindlessIndices.size(), 1Ui64));
    srvDesc.Buffer.StructureByteStride = 0;
    m_materialBufferDescriptor = ResourceManager::it().getShaderResourceView(m_materialBuffer, srvDesc);
}

ComPtr<ID3D12PipelineState> Material::getPSO(PSODesc const& psoDesc)
{
    if (m_psoCache.contains(psoDesc))
    {
        return m_psoCache[psoDesc];
    }

    return createPSO(psoDesc);
}

ComPtr<ID3D12PipelineState> Material::createPSO(PSODesc const& psoDesc)
{
    std::wstring passNameDefine;
    passNameDefine.resize(psoDesc.m_passName.size());
    std::transform(psoDesc.m_passName.begin(), psoDesc.m_passName.end(), passNameDefine.begin(), std::towupper);
    ShaderManager::ShaderDesc vertexShaderDesc =
    {
        .m_filename = psoDesc.m_shaderFilepath,
        .m_entryPoint = psoDesc.m_passName + L"_vs",
        .m_shaderType = ShaderManager::ShaderType::VertexShader,
#if defined(RENDERER_DEBUG)
        .m_enableDebug = true,
#else
        .m_enableDebug = false,
#endif
        .m_defines = {passNameDefine},
    };

    ShaderManager::ShaderDesc pixelShaderDesc =
    {
        .m_filename = psoDesc.m_shaderFilepath,
        .m_entryPoint = psoDesc.m_passName + L"_ps",
        .m_shaderType = ShaderManager::ShaderType::PixelShader,
#if defined(RENDERER_DEBUG)
        .m_enableDebug = true,
#else
        .m_enableDebug = false,
#endif
        .m_defines = {passNameDefine},
    };

    PipelineStateStream pipelineStateStream =
    {
        .s_rootSignature = Renderer::getRootSignature().Get(),
        .m_vertexShader = ShaderManager::it().getShader(vertexShaderDesc),
        .m_pixelShader = ShaderManager::it().getShader(pixelShaderDesc),
        .m_blendState = psoDesc.m_blendState,
        .m_rasterizerState = psoDesc.m_rasterizerState,
        .m_depthStencilState = psoDesc.m_depthStencilState,
        .m_inputLayout = D3D12_INPUT_LAYOUT_DESC{ Mesh::getInputLayout().data(), static_cast<UINT>(Mesh::getInputLayout().size()) },
        .m_topologyType = psoDesc.m_primitiveTopologyType,
        .m_rtFormats = psoDesc.m_rtFormats,
        .m_dsFormat = psoDesc.m_dsFormat,
    };

    ComPtr<ID3D12PipelineState> pso;
    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
    {
        .SizeInBytes = sizeof(PipelineStateStream),
        .pPipelineStateSubobjectStream = &pipelineStateStream,
    };
    Renderer::device()->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pso));

    m_psoCache[psoDesc] = pso;
    m_psoCache[psoDesc]->SetName((m_desc.m_name + L"/" + psoDesc.m_passName).c_str());
    return m_psoCache[psoDesc];
}
