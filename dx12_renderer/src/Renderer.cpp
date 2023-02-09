#include "Renderer.h"

#include <d3dx12.h>

#include <resource/ResourceManager.h>

RECT Renderer::s_windowRect;
ComPtr<ID3D12Device2> Renderer::s_device{ nullptr };
UINT Renderer::s_currentBackBufferIndex = 0;
ComPtr<ID3D12RootSignature> Renderer::s_rootSignature{ nullptr };

Renderer::Renderer( HWND hWnd, RECT windowRect )
    : m_hWnd(hWnd)
{
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

    m_fence = std::make_unique<Fence>( s_device );

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


    // Create RTVs for the backbuffers
    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i )
    {
        ComPtr<ID3D12Resource> backBuffer;
        m_swapChain->GetBuffer( i, IID_PPV_ARGS( &backBuffer ) );
        ResourceManager::it().createResource( L"backbuffer" + std::to_wstring(i), backBuffer );
    }


    // Create root signature
    CD3DX12_ROOT_PARAMETER slotRootParameters[ 5 ] = {};

    slotRootParameters[ 0 ].InitAsConstantBufferView( 0, 0 ); // Camera buffer
    slotRootParameters[ 1 ].InitAsConstantBufferView( 1, 0 ); // Resource indices buffer

    D3D12_DESCRIPTOR_RANGE srvRangeTexture2DHeap{};
    srvRangeTexture2DHeap.BaseShaderRegister = 0;
    srvRangeTexture2DHeap.RegisterSpace = 0;
    srvRangeTexture2DHeap.NumDescriptors = 1024;
    srvRangeTexture2DHeap.OffsetInDescriptorsFromTableStart = 0;
    srvRangeTexture2DHeap.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    slotRootParameters[ 2 ].InitAsDescriptorTable( 1, &srvRangeTexture2DHeap );

    D3D12_DESCRIPTOR_RANGE srvRangeTextureCubeHeap{};
    srvRangeTextureCubeHeap.BaseShaderRegister = 0;
    srvRangeTextureCubeHeap.RegisterSpace = 1;
    srvRangeTextureCubeHeap.NumDescriptors = 1024;
    srvRangeTextureCubeHeap.OffsetInDescriptorsFromTableStart = 0;
    srvRangeTextureCubeHeap.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    slotRootParameters[ 3 ].InitAsDescriptorTable( 1, &srvRangeTextureCubeHeap );

    D3D12_DESCRIPTOR_RANGE srvRangeSamplerHeap{};
    srvRangeSamplerHeap.BaseShaderRegister = 0;
    srvRangeSamplerHeap.RegisterSpace = 0;
    srvRangeSamplerHeap.NumDescriptors = 1024;
    srvRangeSamplerHeap.OffsetInDescriptorsFromTableStart = 0;
    srvRangeSamplerHeap.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    slotRootParameters[ 4 ].InitAsDescriptorTable( 1, &srvRangeSamplerHeap );

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

    Renderer::device()->CreateRootSignature( 0,
                                             serializedRootSig->GetBufferPointer(),
                                             serializedRootSig->GetBufferSize(),
                                             IID_PPV_ARGS( s_rootSignature.GetAddressOf() ) );
}

Renderer::~Renderer()
{
}

void Renderer::addRenderPass( RenderPass& renderPass )
{
    m_renderPasses.push_back( renderPass );
}

void Renderer::drawScene( Scene& scene )
{
    std::vector< ID3D12CommandList* > commandLists;
    for ( RenderPass& renderPass : m_renderPasses )
    {
        renderPass.record( scene );
        commandLists.push_back( renderPass.getCommandList() );
    }
    m_graphicsCmdQueue->ExecuteCommandLists( static_cast<UINT>( commandLists.size() ), commandLists.data() );
    m_fence->GPUSignal( m_graphicsCmdQueue );

    m_fence->CPUWait( m_fence->getLastSignaledValue() );
    m_swapChain->Present( 1, 0 );
    s_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}