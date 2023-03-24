#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

#include <unordered_map>

#include <resource/Descriptor.h>

class Resource;

class Material
{
public:
    struct Technique
    {
        std::wstring m_name;
        std::wstring m_shaderFilename;
        CD3DX12_BLEND_DESC m_blendState = CD3DX12_BLEND_DESC( CD3DX12_DEFAULT{} );
        CD3DX12_RASTERIZER_DESC m_rasterizerState = CD3DX12_RASTERIZER_DESC( CD3DX12_DEFAULT{} );
        CD3DX12_DEPTH_STENCIL_DESC m_depthStencilState = CD3DX12_DEPTH_STENCIL_DESC( CD3DX12_DEFAULT{} );
        D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        CD3DX12_RT_FORMAT_ARRAY m_rtFormats;
        DXGI_FORMAT m_dsFormat;
    };

    struct MaterialDesc
    {
        std::wstring m_name;
        std::vector<Technique> m_techniques;
        std::vector<Descriptor> m_resourceViews;
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    };

    Material( MaterialDesc const& materialDesc );
    ~Material() = default;

    Material( const Material& ) = delete;
    Material& operator= ( const Material& ) = delete;

    std::wstring const& getName() const { return m_desc.m_name; }
    ComPtr<ID3D12PipelineState> getPSOForTechnique( wchar_t const* techniqueName ) const;
    Resource const* getMaterialBufferResource() const { return m_materialBuffer; }
    Resource const* getBindlessIndicesBufferResource() const { return m_bindlessIndicesBuffer; }

    bool hasTechnique( wchar_t const* techniqueName ) const;

private:
    struct MaterialBufferData
    {
        uint32_t bindlessIndicesBufferIndex;
    };
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE m_rootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_VS m_vertexShader;
        CD3DX12_PIPELINE_STATE_STREAM_PS m_pixelShader;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC m_blendState;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER m_rasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL m_depthStencilState;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT m_inputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY m_topologyType;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS m_rtFormats;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT m_dsFormat;

        bool operator==( PipelineStateStream const& other ) const
        {
            return memcmp( this, &other, sizeof( PipelineStateStream ) ) == 0;
        }

        struct Hasher
        {
            size_t operator()( PipelineStateStream const& pipelineStateStream ) const noexcept
            {
                // FNV-1a hash
                std::size_t hash = 0xcbf29ce484222325;
                char const* data = reinterpret_cast<char const*>( &pipelineStateStream );

                for ( int i = 0; i < sizeof( pipelineStateStream ); ++i )
                {
                    hash ^= data[ i ];
                    hash *= 0x100000001b3;
                }

                return hash;
            }
        };
    };

    MaterialDesc m_desc;

    Resource* m_materialBuffer;
    MaterialBufferData m_materialBufferData;

    Resource* m_bindlessIndicesBuffer;
    std::vector<UINT> m_bindlessIndices;
    
    // PSO name is: <technique_name>
    using PSOCache = std::unordered_map< std::wstring, ComPtr<ID3D12PipelineState> >;
    PSOCache m_psoCache;
};