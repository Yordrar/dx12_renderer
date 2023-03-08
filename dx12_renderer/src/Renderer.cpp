#include "Renderer.h"

#include <resource/ResourceManager.h>

RECT Renderer::s_windowRect;
ComPtr<ID3D12Device2> Renderer::s_device{ nullptr };
UINT Renderer::s_currentBackBufferIndex = 0;
uint64_t Renderer::s_timestampFrequency = 0;
ComPtr<ID3D12RootSignature> Renderer::s_rootSignature{ nullptr };

Renderer::Renderer( HWND hWnd, RECT windowRect )
    : m_hWnd(hWnd)
{
    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i ) m_fenceValues[ i ] = 0;
    m_fenceValues[ 0 ] = 1;
    s_currentBackBufferIndex = 0;
    s_windowRect = windowRect;
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugInterface;
    D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) );
    debugInterface->EnableDebugLayer();
#endif

    // Create DXGI factory
    ComPtr<IDXGIFactory7> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    CreateDXGIFactory2( createFactoryFlags, IID_PPV_ARGS( &dxgiFactory ) );

    // Select adapter with greatest amount of memory
    int i = 0;
    ComPtr<IDXGIAdapter1> currentAdapter;
    ComPtr<IDXGIAdapter1> selectedAdapter;
    SIZE_T maxDedicatedVideoMemory = 0;
    while ( dxgiFactory->EnumAdapters1( i, &currentAdapter ) == S_OK )
    {
        DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
        currentAdapter->GetDesc1( &dxgiAdapterDesc );
        if ( ( dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE ) == 0 &&
             SUCCEEDED( D3D12CreateDevice( currentAdapter.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof( ID3D12Device ), nullptr ) ) &&
             dxgiAdapterDesc.DedicatedVideoMemory > maxDedicatedVideoMemory )
        {
            selectedAdapter = currentAdapter;
            maxDedicatedVideoMemory = dxgiAdapterDesc.DedicatedVideoMemory;
        }

        ++i;
    }

    // Create dx12 device
    D3D12CreateDevice( selectedAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS( &s_device ) );

    // Debug break on error
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if ( SUCCEEDED( s_device.As( &pInfoQueue ) ) )
    {
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE );
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, TRUE );
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, FALSE );
    }
#endif

    // Create command queues
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.NodeMask = 0;
    s_device->CreateCommandQueue( &cmdQueueDesc, IID_PPV_ARGS( &m_graphicsCmdQueue ) );

    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    s_device->CreateCommandQueue( &cmdQueueDesc, IID_PPV_ARGS( &m_computeCmdQueue ) );

    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
    s_device->CreateCommandQueue( &cmdQueueDesc, IID_PPV_ARGS( &m_copyCmdQueue ) );

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = s_windowRect.right - s_windowRect.left;
    swapChainDesc.Height = s_windowRect.bottom - s_windowRect.top;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = RendererConstants::sc_numBackBuffers;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    ComPtr<IDXGISwapChain1> swapChain;
    dxgiFactory->CreateSwapChainForHwnd( m_graphicsCmdQueue.Get(), m_hWnd, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf() );
    swapChain.As( &m_swapChain );

    m_frameFence = std::make_unique<Fence>( L"Renderer_frameFence" );

    // Create RTVs for the backbuffers
    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i )
    {
        ComPtr<ID3D12Resource> backBuffer;
        m_swapChain->GetBuffer( i, IID_PPV_ARGS( &backBuffer ) );
        ResourceManager::it().createResource( ( L"backbuffer" + std::to_wstring( i ) ).c_str(), backBuffer );

    }

    // Create root signature
    CD3DX12_ROOT_PARAMETER slotRootParameters[ 7 ] = {};

    slotRootParameters[ 0 ].InitAsConstantBufferView( 0, 0 ); // Scene buffer
    slotRootParameters[ 1 ].InitAsConstantBufferView( 1, 0 ); // Material buffer
    slotRootParameters[ 2 ].InitAsConstantBufferView( 2, 0 ); // Geometry buffer

    D3D12_DESCRIPTOR_RANGE srvRangeBuffer{};
    srvRangeBuffer.BaseShaderRegister = 0;
    srvRangeBuffer.RegisterSpace = 0;
    srvRangeBuffer.NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps;
    srvRangeBuffer.OffsetInDescriptorsFromTableStart = 0;
    srvRangeBuffer.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    slotRootParameters[ 3 ].InitAsDescriptorTable( 1, &srvRangeBuffer );

    D3D12_DESCRIPTOR_RANGE srvRangeTexture2D{};
    srvRangeTexture2D.BaseShaderRegister = 0;
    srvRangeTexture2D.RegisterSpace = 1;
    srvRangeTexture2D.NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps;
    srvRangeTexture2D.OffsetInDescriptorsFromTableStart = 0;
    srvRangeTexture2D.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    slotRootParameters[ 4 ].InitAsDescriptorTable( 1, &srvRangeTexture2D );

    D3D12_DESCRIPTOR_RANGE srvRangeTextureCube{};
    srvRangeTextureCube.BaseShaderRegister = 0;
    srvRangeTextureCube.RegisterSpace = 2;
    srvRangeTextureCube.NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps;
    srvRangeTextureCube.OffsetInDescriptorsFromTableStart = 0;
    srvRangeTextureCube.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    slotRootParameters[ 5 ].InitAsDescriptorTable( 1, &srvRangeTextureCube );

    D3D12_DESCRIPTOR_RANGE srvRangeSampler{};
    srvRangeSampler.BaseShaderRegister = 0;
    srvRangeSampler.RegisterSpace = 0;
    srvRangeSampler.NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps;
    srvRangeSampler.OffsetInDescriptorsFromTableStart = 0;
    srvRangeSampler.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    slotRootParameters[ 6 ].InitAsDescriptorTable( 1, &srvRangeSampler );

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc( _countof(slotRootParameters), slotRootParameters, 0, nullptr,
                                             D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                             D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                             D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                                             D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                             D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
                                             D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS );

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature( &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                              serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf() );

    if ( errorBlob != nullptr && errorBlob->GetBufferSize() != 0 )
    {
        OutputDebugStringA( (LPCSTR)errorBlob->GetBufferPointer() );
    }

    Renderer::device()->CreateRootSignature( 0,
                                             serializedRootSig->GetBufferPointer(),
                                             serializedRootSig->GetBufferSize(),
                                             IID_PPV_ARGS( s_rootSignature.GetAddressOf() ) );

    m_graphicsCmdQueue->GetTimestampFrequency( &s_timestampFrequency );
}

static auto start = std::chrono::high_resolution_clock::now();
static std::vector<ID3D12CommandList*> commandLists;
static double gpuTime = 0;
void Renderer::beginFrame()
{
    commandLists.clear();

    gpuTime = 0;

    start = std::chrono::high_resolution_clock::now();
}

void Renderer::submitPass( RenderPass& pass )
{
    pass.record();
    gpuTime += pass.getExecutionTimeMilliseconds();
    commandLists.push_back( pass.getCommandList() );
}

void Renderer::endFrame()
{
    m_graphicsCmdQueue->ExecuteCommandLists( static_cast<UINT>( commandLists.size() ), commandLists.data() );
    m_swapChain->Present( 0, 0 );

    // Schedule a Signal command in the queue.
    uint64_t const currentFenceValue = m_fenceValues[ s_currentBackBufferIndex ];
    m_frameFence->GPUSignal( m_graphicsCmdQueue, currentFenceValue );

    // Update the frame index.
    s_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    m_frameFence->CPUWait( m_fenceValues[ s_currentBackBufferIndex ] );

    // Set the fence value for the next frame.
    m_fenceValues[ s_currentBackBufferIndex ] = currentFenceValue + 1;

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( end - start );
    OutputDebugStringA( std::string( "CPU time: " + std::to_string( elapsed.count() ) + "ms\n" ).c_str() );
    OutputDebugStringA( std::string( "GPU time: " + std::to_string( gpuTime ) + "ms\n" ).c_str() );
}

void Renderer::waitForIdleGPU()
{
    uint64_t const currentFenceValue = m_fenceValues[ s_currentBackBufferIndex ];
    m_frameFence->GPUSignal( m_graphicsCmdQueue, currentFenceValue );
    m_frameFence->CPUWait( m_fenceValues[ s_currentBackBufferIndex ] );
    m_fenceValues[ s_currentBackBufferIndex ]++;
}

UINT Renderer::getPreviousBackbufferIndex()
{
    return std::clamp( s_currentBackBufferIndex - 1, 0u, RendererConstants::sc_numBackBuffers - 1 );
}
