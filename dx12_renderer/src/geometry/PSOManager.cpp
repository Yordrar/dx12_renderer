#include "PSOManager.h"

#include <Renderer.h>

PSOManager::PSOManager()
{
    Renderer::device()->CreatePipelineLibrary( nullptr, 0, IID_PPV_ARGS( &m_PSOLibrary ) );
}

ComPtr<ID3D12PipelineState> PSOManager::getPSO( std::wstring PSOName, PipelineStateStream& PSODesc )
{
    ComPtr<ID3D12PipelineState> pso;
    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof( PipelineStateStream ), &PSODesc
    };
    HRESULT result = m_PSOLibrary->LoadPipeline( PSOName.c_str(), &pipelineStateStreamDesc, IID_PPV_ARGS( &pso ) );

    if ( result != S_OK )
    {
        Renderer::device()->CreatePipelineState( &pipelineStateStreamDesc, IID_PPV_ARGS( &pso ) );
        m_PSOLibrary->StorePipeline( PSOName.c_str(), pso.Get() );
    }

    return pso;
}