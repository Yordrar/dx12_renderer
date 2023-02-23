#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>
#include <unordered_map>

#include <Manager.h>

class PSOManager : public Manager<PSOManager>
{
    friend class Manager<PSOManager>;
public:
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

    ~PSOManager() = default;

    ComPtr<ID3D12PipelineState> getPSO( PipelineStateStream& PSODesc );

private:
    PSOManager();

    ComPtr<ID3D12PipelineLibrary1> m_psoLibrary;

    using PSOCache = std::unordered_map< PipelineStateStream, ComPtr<ID3D12PipelineState>, PipelineStateStream::Hasher >;
    PSOCache m_psoCache;
};