#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>

#include <Manager.h>

class PSOManager : public Manager<PSOManager>
{
    friend class Manager<PSOManager>;
public:
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE m_rootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT m_inputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY m_topologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS m_vertexShader;
        CD3DX12_PIPELINE_STATE_STREAM_PS m_pixelShader;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT m_dsFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS m_rtFormats;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER m_rasterizer;
    };

    ~PSOManager() = default;

    ComPtr<ID3D12PipelineState> getPSO( std::wstring PSOName, PipelineStateStream& PSODesc );

private:
    PSOManager();

    ComPtr<ID3D12PipelineLibrary1> m_PSOLibrary;
};