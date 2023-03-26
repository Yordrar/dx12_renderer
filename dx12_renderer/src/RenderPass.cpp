#include "RenderPass.h"

#include <Renderer.h>
#include <Profiler.h>
#include <resource/ResourceManager.h>
#include <resource/Descriptor.h>
#include <geometry/MaterialManager.h>

RenderPass::RenderPass( wchar_t const* name,
                        wchar_t const* techniqueName,
                        wchar_t const* renderTargetName,
                        wchar_t const* depthStencilTargetName )
    : m_name( name )
    , m_techniqueName( techniqueName )
    , m_commandList( nullptr )
    , m_renderTargetName( renderTargetName )
    , m_depthStencilTargetName( depthStencilTargetName )
    , m_renderTarget( nullptr )
    , m_depthStencilTarget( nullptr )
    , m_scissorRect( Renderer::getWindowRect() )
    , m_profilerQueryIndex( Profiler::it().allocateQueryIndex() )
    , m_executionTimeInMilliseconds( 0 )
    , m_passBuffer( nullptr )
    , m_passResourceIndicesBuffer( nullptr )
{
    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i )
    {
        Renderer::device()->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_commandAllocators[ i ] ) );
    }

    HRESULT result = Renderer::device()->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[ 0 ].Get(), nullptr, IID_PPV_ARGS( &m_commandList ) );
    m_commandList->Close();
    std::wstring commandListName = m_name + L"_commandList";
    m_commandList->SetName( commandListName.c_str() );

    if ( m_renderTargetName != L"backbuffer" )
    {
        m_renderTarget = ResourceManager::it().getResource( m_renderTargetName.c_str() );
    }
    m_depthStencilTarget = ResourceManager::it().getResource( m_depthStencilTargetName.c_str() );
}

RenderPass::~RenderPass()
{

}

void RenderPass::record( Scene const& scene, std::vector<Camera*> const& cameras )
{
    // Reset command list and allocator
    ComPtr<ID3D12CommandAllocator> currentCommandAllocator = m_commandAllocators[ Renderer::getCurrentBackbufferIndex() ];
    currentCommandAllocator->Reset();
    m_commandList->Reset( currentCommandAllocator.Get(), nullptr );

    static FLOAT clearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Resource* renderTarget = nullptr;
    if ( m_renderTargetName == L"backbuffer" )
    {
        renderTarget = ResourceManager::it().getCurrentBackbufferResource();
    }

    if ( !m_passResourceIndicesBuffer )
    {
        m_passResourceIndicesBuffer = ResourceManager::it().createResource( ( m_name + L"_passResourceIndicesBuffer" ).c_str(),
                                                                            CD3DX12_RESOURCE_DESC::Buffer( std::max( m_passResourceIndicesBufferData.size() * sizeof( UINT ), 1Ui64 ) ),
                                                                            D3D12_SUBRESOURCE_DATA{ m_passResourceIndicesBufferData.data(), static_cast<LONG_PTR>( m_passResourceIndicesBufferData.size() * sizeof( UINT ) ), 0 } );
    }
    if ( !m_passBuffer )
    {
        m_passBufferData.passResourceIndicesBufferIndex = m_passResourceIndicesBuffer->getShaderResourceView()->getDescriptorIndex();
        m_passBuffer = ResourceManager::it().createResource( ( m_name + L"_passBuffer" ).c_str(),
                                                             CD3DX12_RESOURCE_DESC::Buffer( std::max( sizeof( m_passBufferData ), 1Ui64 ) ),
                                                             D3D12_SUBRESOURCE_DATA{ &m_passBufferData, static_cast<LONG_PTR>( sizeof( m_passBufferData ) ), 0 } );
    }

    Profiler::it().startQuery( m_commandList.Get(), m_profilerQueryIndex );

    ResourceManager::it().copyResourcesToGPU( m_commandList );

    PIXBeginEvent( m_commandList.Get(), PIX_COLOR_DEFAULT, m_name.c_str() );

    // Set viewport
    if ( renderTarget )
    {
        D3D12_VIEWPORT viewport = CD3DX12_VIEWPORT( renderTarget->getResource().Get() );
        m_commandList->RSSetViewports( 1, &viewport );
    }
    m_commandList->RSSetScissorRects( 1, &m_scissorRect );

    // Set root signature
    m_commandList->SetGraphicsRootSignature( Renderer::getRootSignature().Get() );

    // Set descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] =
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get(),
        DescriptorHeap::getDescriptorHeapSampler().getHeap().Get(),
    };
    m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

    // Set descriptor tables in root signature
    m_commandList->SetGraphicsRootDescriptorTable( 4, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 5, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 6, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 7, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 8, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 9, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 10, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 11, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 12, DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 13, DescriptorHeap::getDescriptorHeapSampler().getHeap()->GetGPUDescriptorHandleForHeapStart() );

    // Clear and set render targets
    std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
    if ( renderTarget && renderTarget->getResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET )
    {
        CD3DX12_RESOURCE_BARRIER renderTargetBarrier = renderTarget->getTransitionBarrier( D3D12_RESOURCE_STATE_RENDER_TARGET );
        barriers.push_back( renderTargetBarrier );
    }
    if ( m_depthStencilTarget && m_depthStencilTarget->getResourceState() != D3D12_RESOURCE_STATE_DEPTH_WRITE )
    {
        CD3DX12_RESOURCE_BARRIER depthStencilBarrier = m_depthStencilTarget->getTransitionBarrier( D3D12_RESOURCE_STATE_DEPTH_WRITE );
        barriers.push_back( depthStencilBarrier );
    }
    if ( barriers.size() > 0 )
    {
        m_commandList->ResourceBarrier( static_cast<UINT>( barriers.size() ), barriers.data() );
    }

    if ( renderTarget )
    {
        m_commandList->ClearRenderTargetView( renderTarget->getRenderTargetView()->getView(), clearColor, 0, nullptr );
    }
    if ( m_depthStencilTarget )
    {
        m_commandList->ClearDepthStencilView( m_depthStencilTarget->getDepthStencilView()->getView(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_depthStencilTarget->getDepthStencilView()->getView();
    if ( renderTarget )
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = renderTarget->getRenderTargetView()->getView();
        m_commandList->OMSetRenderTargets( 1, &rtv, false, &dsv );
    }
    else
    {
        m_commandList->OMSetRenderTargets( 0, nullptr, false, &dsv );
    }

    // Record scenes
    std::wstring currentMaterialName;
    m_commandList->SetGraphicsRootConstantBufferView( 0, m_passBuffer->getGPUVirtualAddress() );
    for ( std::shared_ptr<Mesh> const& currentMesh : scene.getMeshes() )
    {
        if ( !currentMesh->hasVertexBuffer() )
        {
            continue;
        }

        for ( Camera* camera : cameras )
        {
            m_commandList->SetGraphicsRootConstantBufferView( 1, camera->getGPUBufferResource()->getGPUVirtualAddress() );
            if ( currentMesh->isAABBValid() && !camera->isAABBVisible( currentMesh->getAABB() ) )
            {
                continue;
            }

            std::wstring const& currentMeshMaterialName = currentMesh->getMaterialName();
            if ( currentMaterialName != currentMeshMaterialName )
            {
                Material* currentMeshMaterial = MaterialManager::it().getMaterial( currentMeshMaterialName.c_str() );
                if ( currentMeshMaterial && currentMeshMaterial->hasTechnique( m_techniqueName.c_str() ) )
                {
                    m_commandList->SetGraphicsRootConstantBufferView( 2, currentMeshMaterial->getMaterialBufferResource()->getGPUVirtualAddress() );
                    m_commandList->SetPipelineState( currentMeshMaterial->getPSOForTechnique( m_techniqueName.c_str() ).Get() );
                    currentMaterialName = currentMeshMaterialName;
                }
                else
                {
                    continue;
                }
            }

            currentMesh->record( m_commandList );
        }
    }

    PIXEndEvent( m_commandList.Get() );

    Profiler::it().endQuery( m_commandList.Get(), m_profilerQueryIndex );
    m_executionTimeInMilliseconds = Profiler::it().getResolvedQuery( m_profilerQueryIndex );

    m_commandList->Close();
}

void RenderPass::addResourceView( Descriptor const& descriptor )
{
    m_passResourceIndicesBufferData.push_back( descriptor.getDescriptorIndex() );
}

void RenderPass::addComputePassToWaitOn( ComputePass* computePass )
{
    m_computePassesToWaitOn.push_back( computePass );
    m_computePassFenceCounters.push_back( 0 );
}

void RenderPass::waitOnComputePasses( ComPtr<ID3D12CommandQueue> cmdQueue, std::vector<ComputePass*> const& submittedComputePasses )
{
    for ( size_t i = 0; i < m_computePassesToWaitOn.size(); ++i )
    {
        if ( std::find( submittedComputePasses.begin(), submittedComputePasses.end(), m_computePassesToWaitOn[ i ] ) != submittedComputePasses.end() )
        {
            m_computePassesToWaitOn[ i ]->getFence().GPUWait( cmdQueue, ++m_computePassFenceCounters[ i ] );
        }
    }
}
