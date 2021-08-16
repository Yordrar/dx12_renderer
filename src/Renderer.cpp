#include "Renderer.h"

#include <d3dx12.h>

Renderer::Renderer( HWND hWnd, RECT windowRect )
    : m_hWnd(hWnd)
    , m_windowRect(windowRect)
{
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

    // Select adapter with greatest amount of mem
    int i = 0;
    ComPtr<IDXGIAdapter1> currentAdapter;
    ComPtr<IDXGIAdapter1> selectedAdapter;
    SIZE_T maxDedicatedVideoMemory = 0;
    while ( dxgiFactory->EnumAdapters1( i, &currentAdapter ) == S_OK )
    {
        DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
        currentAdapter->GetDesc1( &dxgiAdapterDesc );
        if ( ( dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE ) == 0 &&
             SUCCEEDED( D3D12CreateDevice( currentAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof( ID3D12Device ), nullptr ) ) &&
             dxgiAdapterDesc.DedicatedVideoMemory > maxDedicatedVideoMemory )
        {
            selectedAdapter = currentAdapter;
            maxDedicatedVideoMemory = dxgiAdapterDesc.DedicatedVideoMemory;
        }

        ++i;
    }

    // Create dx12 device
    D3D12CreateDevice( selectedAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_device ) );

    // Debug break on error
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if ( SUCCEEDED( m_device.As( &pInfoQueue ) ) )
    {
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE );
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, TRUE );
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, TRUE );
    }
#endif

    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.NodeMask = 0;
    m_device->CreateCommandQueue( &cmdQueueDesc, IID_PPV_ARGS( &m_commandQueue ) );

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_windowRect.right - m_windowRect.left;
    swapChainDesc.Height = m_windowRect.bottom - m_windowRect.top;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = sc_numBackBuffers;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    dxgiFactory->CreateSwapChainForHwnd( m_commandQueue.Get(), m_hWnd, &swapChainDesc, nullptr, nullptr, m_swapChain.GetAddressOf() );

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = sc_numBackBuffers;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    m_device->CreateDescriptorHeap( &heapDesc, IID_PPV_ARGS( &m_RTVDescriptorHeap ) );

    auto rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart() );
    for ( int i = 0; i < sc_numBackBuffers; ++i )
    {
        ComPtr<ID3D12Resource> backBuffer;
        m_swapChain->GetBuffer( i, IID_PPV_ARGS( &backBuffer ) );
        m_device->CreateRenderTargetView( backBuffer.Get(), nullptr, rtvHandle );
        m_backBuffers[ i ] = backBuffer;
        rtvHandle.Offset( rtvDescriptorSize );

        m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_commandAllocators[ i ] ) );
    }

    m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[ 0 ].Get(), nullptr, IID_PPV_ARGS( &m_commandList ) );
    m_commandList->Close();
}

Renderer::~Renderer()
{
}
