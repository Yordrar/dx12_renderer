#include "ComputePass.h"

#include <Renderer.h>
#include <Profiler.h>
#include <BarrierRecorder.h>
#include <resource/ResourceManager.h>
#include <resource/Descriptor.h>
#include <geometry/ShaderManager.h>

ComputePass::ComputePass( wchar_t const* name,
                          wchar_t const* shaderFilename,
                          UINT threadGroupCountX,
                          UINT threadGroupCountY,
                          UINT threadGroupCountZ )
    : m_name( name )
    , m_threadGroupCountX( threadGroupCountX )
    , m_threadGroupCountY( threadGroupCountY )
    , m_threadGroupCountZ( threadGroupCountZ )
    , m_commandList( nullptr )
    , m_profilerQueryIndex( 0 )
    , m_executionTimeInMilliseconds( 0 )
    , m_fence( (m_name + L"_fence").c_str() )
{
    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i )
    {
        Renderer::device()->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS( &m_commandAllocators[ i ] ) );
    }

    HRESULT result = Renderer::device()->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_commandAllocators[ 0 ].Get(), nullptr, IID_PPV_ARGS( &m_commandList ) );
    m_commandList->Close();
    std::wstring commandListName = m_name + L"_commandList";
    m_commandList->SetName( commandListName.c_str() );

    std::wstring passNameDefine;
    passNameDefine.resize( m_name.size() );
    std::transform( m_name.begin(), m_name.end(), passNameDefine.begin(), std::towupper );
    ShaderManager::ShaderDesc computeShaderDesc =
    {
        .m_filename = shaderFilename,
        .m_entryPoint = m_name + L"_cs",
        .m_shaderType = ShaderManager::ShaderType::ComputeShader,
        .m_enableDebug = true,
        .m_defines = {passNameDefine},
    };
    PipelineStateStream pipelineStateStream =
    {
        .m_rootSignature = Renderer::getRootSignature().Get(),
        .m_computeShader = ShaderManager::it().getShader( computeShaderDesc ),
    };
    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
    {
        .SizeInBytes = sizeof( PipelineStateStream ),
        .pPipelineStateSubobjectStream = &pipelineStateStream,
    };
    Renderer::device()->CreatePipelineState( &pipelineStateStreamDesc, IID_PPV_ARGS( &m_pso ) );
}

ComputePass::~ComputePass()
{

}

void ComputePass::record()
{
    // Reset command list and allocator
    ComPtr<ID3D12CommandAllocator> currentCommandAllocator = m_commandAllocators[ Renderer::getCurrentBackbufferIndex() ];
    currentCommandAllocator->Reset();
    m_commandList->Reset( currentCommandAllocator.Get(), nullptr );

    if ( !m_passBuffer.isValid() )
    {
        m_passBuffer = ResourceManager::it().createResource( ( m_name + L"_passBuffer" ).c_str(),
                                                                            CD3DX12_RESOURCE_DESC::Buffer( std::max( m_passBufferData.size() * sizeof( UINT ), 1Ui64 ) ),
                                                                            D3D12_SUBRESOURCE_DATA{ m_passBufferData.data(), static_cast<LONG_PTR>( m_passBufferData.size() * sizeof( UINT ) ), 0 } );
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
        {
            .Format = DXGI_FORMAT_R32_TYPELESS,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        };
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Buffer.NumElements = static_cast<UINT>( m_passBufferData.size() );
        srvDesc.Buffer.StructureByteStride = 0;
        m_passBufferDescriptor = ResourceManager::it().getShaderResourceView( m_passBuffer, srvDesc );
    }

    ResourceManager::it().copyResourcesToGPU( m_commandList );

    PIXBeginEvent( m_commandList.Get(), PIX_COLOR_DEFAULT, m_name.c_str() );

    // Set PSO
    m_commandList->SetPipelineState( m_pso.Get() );

    // Set descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] =
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get(),
        DescriptorHeap::getDescriptorHeapSampler().getHeap().Get(),
    };
    m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

    // Set root signature
    m_commandList->SetComputeRootSignature(Renderer::getRootSignature().Get());

    m_commandList->SetComputeRoot32BitConstant( 0, m_passBufferDescriptor.getDescriptorIndex(), 0 );

    // Transition UAV resources
    BarrierRecorder br;
    for ( Descriptor const& descriptor : m_resourceViews )
    {
        if ( descriptor.getType() == Descriptor::Type::UnorderedAccessView )
        {
            br.recordBarrierTransition(descriptor.getResourceHandle(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        }
    }
    br.submitBarriers(m_commandList);

    m_commandList->Dispatch( m_threadGroupCountX, m_threadGroupCountY, m_threadGroupCountZ );

    for ( Descriptor const& descriptor : m_resourceViews )
    {
        if ( descriptor.getType() == Descriptor::Type::UnorderedAccessView )
        {
            br.recordBarrierUAV(descriptor.getResourceHandle());
        }
    }
    br.submitBarriers(m_commandList);

    PIXEndEvent( m_commandList.Get() );

    m_commandList->Close();
}

void ComputePass::addResourceView( Descriptor const descriptor )
{
    m_passBufferData.push_back( descriptor.getDescriptorIndex() );
    m_resourceViews.push_back( descriptor );
}

void ComputePass::setResourceView( UINT index, Descriptor const descriptor )
{
    assert( index >= 0 && index < m_passBufferData.size() );
    assert( index >= 0 && index < m_resourceViews.size() );
    m_passBufferData[ index ] = descriptor.getDescriptorIndex();
    m_resourceViews[ index ] = descriptor;
    ResourceManager::it().setResourceNeedsCopyToGPU(m_passBuffer);
}

void ComputePass::setThreadGroupCounts( UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ )
{
    setThreadGroupCountX( threadGroupCountX );
    setThreadGroupCountY( threadGroupCountY );
    setThreadGroupCountZ( threadGroupCountZ );
}

void ComputePass::transitionResourcesForNextFrame( ComPtr<ID3D12GraphicsCommandList> commandList )
{
    BarrierRecorder br;
    for( Descriptor const& descriptor : m_resourceViews )
    {
        br.recordBarrierTransition(descriptor.getResourceHandle(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
    br.submitBarriers(commandList);
}
