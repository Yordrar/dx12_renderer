#pragma once

#include <d3d12.h>
#include <d3dx12/d3dx12.h>

#include <vector>

#include <resource/Descriptor.h>

class Resource;

class Material
{
public:
    struct MaterialDesc
    {
        std::wstring m_name;
        std::vector<Descriptor> m_resourceViews;
    };

    struct PSODesc
    {
        std::wstring m_techniqueName;
        std::wstring m_shaderFilepath;
        D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveTopologyType;
        CD3DX12_RASTERIZER_DESC m_rasterizerState;
        CD3DX12_DEPTH_STENCIL_DESC m_depthStencilState;
        CD3DX12_BLEND_DESC m_blendState;
        CD3DX12_RT_FORMAT_ARRAY m_rtFormats;
        DXGI_FORMAT m_dsFormat;

        bool operator==(PSODesc const& other) const
        {
            return (m_primitiveTopologyType == other.m_primitiveTopologyType &&
                m_rtFormats.NumRenderTargets == other.m_rtFormats.NumRenderTargets &&
                m_dsFormat == other.m_dsFormat &&
                memcmp( &m_rasterizerState, &other.m_rasterizerState, sizeof(CD3DX12_RASTERIZER_DESC) ) == 0 &&
                memcmp( &m_depthStencilState, &other.m_depthStencilState, sizeof(CD3DX12_DEPTH_STENCIL_DESC) ) == 0 &&
                memcmp( &m_blendState, &other.m_blendState, sizeof(CD3DX12_BLEND_DESC) ) == 0 &&
                memcmp( m_rtFormats.RTFormats, other.m_rtFormats.RTFormats, sizeof(m_rtFormats.RTFormats) ) == 0 &&
                m_techniqueName == other.m_techniqueName &&
                m_shaderFilepath == other.m_shaderFilepath);
        }
    };

    Material( MaterialDesc const& materialDesc );
    ~Material();

    Material( const Material& ) = delete;
    Material& operator= ( const Material& ) = delete;

    void addResourceView(Descriptor descriptor);

    std::wstring const& getName() const { return m_desc.m_name; }
    ComPtr<ID3D12PipelineState> getPSO( PSODesc const& psoDesc );
    ResourceHandle getMaterialBufferResource() const { return m_materialBuffer; }
    Descriptor getMaterialBufferDescriptor() const { return m_materialBufferDescriptor; }
    std::vector<Descriptor> const& getResourceViews() const { return m_desc.m_resourceViews; }

private:
    void recreateInternalBuffers();

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE s_rootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_VS m_vertexShader;
        CD3DX12_PIPELINE_STATE_STREAM_PS m_pixelShader;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC m_blendState;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER m_rasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL m_depthStencilState;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT m_inputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY m_topologyType;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS m_rtFormats;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT m_dsFormat;
    };

    struct PSOElement
    {
        PSODesc m_desc;
        ComPtr<ID3D12PipelineState> m_pso;
    };

    ComPtr<ID3D12PipelineState> createPSO(PSODesc const& psoDesc);

    MaterialDesc m_desc;

    ResourceHandle m_materialBuffer;
    std::vector<UINT> m_bindlessIndices;
    Descriptor m_materialBufferDescriptor;
    
    using PSOCache = std::vector<PSOElement>;
    PSOCache m_psoCache;
};