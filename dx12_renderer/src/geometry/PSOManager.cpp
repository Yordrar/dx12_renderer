#include "PSOManager.h"

#include <Renderer.h>

PSOManager::PSOManager()
{
    Renderer::device()->CreatePipelineLibrary( nullptr, 0, IID_PPV_ARGS( &m_psoLibrary ) );
}

ComPtr<ID3D12PipelineState> PSOManager::getPSO( PipelineStateStream& PSODesc )
{
    PSOCache::iterator it = m_psoCache.find( PSODesc );
    if ( it != m_psoCache.end() )
    {
        return it->second;
    }

    ComPtr<ID3D12PipelineState> pso;
    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof( PipelineStateStream ), &PSODesc
    };
    //HRESULT result = m_psoLibrary->LoadPipeline( PSOName.c_str(), &pipelineStateStreamDesc, IID_PPV_ARGS( &pso ) );

    //if ( result != S_OK )
    {
        Renderer::device()->CreatePipelineState( &pipelineStateStreamDesc, IID_PPV_ARGS( &pso ) );
        //m_psoLibrary->StorePipeline( PSOName.c_str(), pso.Get() );
        m_psoCache[ PSODesc ] = pso;
    }

    return pso;
}