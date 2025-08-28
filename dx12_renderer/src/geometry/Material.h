#pragma once

#include <d3d12.h>
#include <d3dx12/d3dx12.h>
#include <dxgi1_6.h>

#include <unordered_map>

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
        std::wstring m_passName;
        std::wstring m_shaderFilepath;
        D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveTopologyType;
        CD3DX12_RASTERIZER_DESC m_rasterizerState;
        CD3DX12_DEPTH_STENCIL_DESC m_depthStencilState;
        CD3DX12_BLEND_DESC m_blendState;
        CD3DX12_RT_FORMAT_ARRAY m_rtFormats;
        DXGI_FORMAT m_dsFormat;

        bool operator==(PSODesc const& other) const
        {
            return (m_passName == other.m_passName &&
                m_shaderFilepath == other.m_shaderFilepath &&
                m_primitiveTopologyType == other.m_primitiveTopologyType &&
                memcmp( &m_rasterizerState, &other.m_rasterizerState, sizeof(CD3DX12_RASTERIZER_DESC) ) == 0 &&
                memcmp( &m_depthStencilState, &other.m_depthStencilState, sizeof(CD3DX12_DEPTH_STENCIL_DESC) ) == 0 &&
                memcmp( &m_blendState, &other.m_blendState, sizeof(CD3DX12_BLEND_DESC) ) == 0 &&
                memcmp( m_rtFormats.RTFormats, other.m_rtFormats.RTFormats, sizeof(m_rtFormats.RTFormats) ) == 0 &&
                m_rtFormats.NumRenderTargets == other.m_rtFormats.NumRenderTargets &&
                m_dsFormat == other.m_dsFormat );
        }

        struct Hasher
        {
            size_t operator()(PSODesc const& psoDesc) const noexcept
            {
                std::size_t hash = 0;
                hash = hash ^ (std::hash<std::wstring>{}(psoDesc.m_passName) << 1);
                hash = hash ^ (std::hash<std::wstring>{}(psoDesc.m_shaderFilepath) << 1);
                uint32_t const* psoDescPtr = reinterpret_cast<uint32_t const*>(&psoDesc.m_primitiveTopologyType);
                for (int i = 0; i < ((sizeof(PSODesc) - (sizeof(std::wstring) * 2)) / sizeof(uint32_t)); i++, psoDescPtr++)
                {
                    hash = hash ^ (std::hash<uint32_t>{}(*psoDescPtr) << 1);
                }
                return hash;
            }
        };
    };
    static_assert(sizeof(PSODesc) % sizeof(uint32_t) == 0, "PSODesc size must be aligned to 4 bytes");

    Material( MaterialDesc const& materialDesc );
    ~Material() = default;

    Material( const Material& ) = delete;
    Material& operator= ( const Material& ) = delete;

    std::wstring const& getName() const { return m_desc.m_name; }
    ComPtr<ID3D12PipelineState> getPSO( PSODesc const& psoDesc );
    ResourceHandle getMaterialBufferResource() const { return m_materialBuffer; }
    Descriptor getMaterialBufferDescriptor() const { return m_materialBufferDescriptor; }
    std::vector<Descriptor> const& getResourceViews() const { return m_desc.m_resourceViews; }

private:
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

    ComPtr<ID3D12PipelineState> createPSO(PSODesc const& psoDesc);

    MaterialDesc m_desc;

    ResourceHandle m_materialBuffer;
    std::vector<UINT> m_bindlessIndices;
    Descriptor m_materialBufferDescriptor;
    
    using PSOCache = std::unordered_map< PSODesc, ComPtr<ID3D12PipelineState>, PSODesc::Hasher >;
    PSOCache m_psoCache;
};