#include "Renderer.h"

#include <d3dx12.h>

#include <resource/ResourceManager.h>

ComPtr<ID3D12Device2> Renderer::m_device{ nullptr };

Renderer::Renderer( HWND hWnd, RECT windowRect )
    : m_hWnd(hWnd)
    , m_windowRect(windowRect)
    , m_currentBackBufferIndex(0)
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
    D3D12CreateDevice( selectedAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS( &m_device ) );

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

    // Create command queues
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.NodeMask = 0;
    m_device->CreateCommandQueue( &cmdQueueDesc, IID_PPV_ARGS( &m_graphicsCmdQueue ) );

    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    m_device->CreateCommandQueue( &cmdQueueDesc, IID_PPV_ARGS( &m_computeCmdQueue ) );

    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
    m_device->CreateCommandQueue( &cmdQueueDesc, IID_PPV_ARGS( &m_copyCmdQueue ) );

    // Create swap chain
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
    ComPtr<IDXGISwapChain1> swapChain;
    dxgiFactory->CreateSwapChainForHwnd( m_graphicsCmdQueue.Get(), m_hWnd, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf() );
    swapChain.As( &m_swapChain );


    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil = { 1.0f, 0 };
    // Create RTVs, DSVs and command allocators for each backbuffer
    for ( int i = 0; i < sc_numBackBuffers; ++i )
    {
        ComPtr<ID3D12Resource> backBuffer;
        m_swapChain->GetBuffer( i, IID_PPV_ARGS( &m_backBuffers[ i ] ) );
        ResourceManager::it()->getRtvDescriptorHeap()->addRTV( m_backBuffers[ i ], nullptr );

        m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, swapChainDesc.Width, swapChainDesc.Height,
                                           1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS( m_depthBuffers[i].GetAddressOf() )
        );
        ResourceManager::it()->getDsvDescriptorHeap()->addDSV( m_depthBuffers[i], nullptr );

        m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_commandAllocators[ i ] ) );
    }


    // Create command list
    m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[ 0 ].Get(), nullptr, IID_PPV_ARGS( &m_commandList ) );
    m_commandList->Close();


    CD3DX12_ROOT_PARAMETER slotRootParameters[ 6 ] = {};

    slotRootParameters[ 0 ].InitAsConstantBufferView( 0, 0 ); // Camera buffer
    slotRootParameters[ 1 ].InitAsConstantBufferView( 1, 0 ); // Bindless indices

    // Bindless resources
    D3D12_DESCRIPTOR_RANGE srvRangeBufferHeap{};
    srvRangeBufferHeap.BaseShaderRegister = 0;
    srvRangeBufferHeap.NumDescriptors = 128;
    srvRangeBufferHeap.OffsetInDescriptorsFromTableStart = 0;
    srvRangeBufferHeap.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRangeBufferHeap.RegisterSpace = 0;
    slotRootParameters[ 2 ].InitAsDescriptorTable( 1, &srvRangeBufferHeap );

    D3D12_DESCRIPTOR_RANGE srvRangeTexture2DHeap{};
    srvRangeTexture2DHeap.BaseShaderRegister = 0;
    srvRangeTexture2DHeap.NumDescriptors = 128;
    srvRangeTexture2DHeap.OffsetInDescriptorsFromTableStart = 0;
    srvRangeTexture2DHeap.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRangeTexture2DHeap.RegisterSpace = 1;
    slotRootParameters[ 3 ].InitAsDescriptorTable( 1, &srvRangeTexture2DHeap );

    D3D12_DESCRIPTOR_RANGE srvRangeTextureCubeHeap{};
    srvRangeTextureCubeHeap.BaseShaderRegister = 0;
    srvRangeTextureCubeHeap.NumDescriptors = 128;
    srvRangeTextureCubeHeap.OffsetInDescriptorsFromTableStart = 0;
    srvRangeTextureCubeHeap.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRangeTextureCubeHeap.RegisterSpace = 2;
    slotRootParameters[ 4 ].InitAsDescriptorTable( 1, &srvRangeTextureCubeHeap );

    D3D12_DESCRIPTOR_RANGE srvRangeSamplerHeap{};
    srvRangeSamplerHeap.BaseShaderRegister = 0;
    srvRangeSamplerHeap.NumDescriptors = 128;
    srvRangeSamplerHeap.OffsetInDescriptorsFromTableStart = 0;
    srvRangeSamplerHeap.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    srvRangeSamplerHeap.RegisterSpace = 0;
    slotRootParameters[ 5 ].InitAsDescriptorTable( 1, &srvRangeSamplerHeap );

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
                                             IID_PPV_ARGS( m_rootSignature.GetAddressOf() ) );
}

Renderer::~Renderer()
{
}

void Renderer::addRenderPass( RenderPass& renderPass )
{
    m_renderPasses.push_back( renderPass );
    m_commandAllocators[ m_currentBackBufferIndex ]->Reset();
    m_commandList->Reset( m_commandAllocators[ m_currentBackBufferIndex ].Get(), nullptr );

    // Set viewport
    m_commandList->RSSetViewports( 1, &CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( m_windowRect.right - m_windowRect.left ), static_cast<float>( m_windowRect.bottom - m_windowRect.top ) ) );
    m_commandList->RSSetScissorRects( 1, &CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) );

    // Set render target and depth buffer
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv( ResourceManager::it()->getRtvDescriptorHeap()->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                       m_currentBackBufferIndex,
                                       ResourceManager::it()->getRtvDescriptorHeap()->getIncrementSize() );
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv( ResourceManager::it()->getDsvDescriptorHeap()->getHeap()->GetCPUDescriptorHandleForHeapStart(),
                                       m_currentBackBufferIndex,
                                       ResourceManager::it()->getDsvDescriptorHeap()->getIncrementSize() );
    m_commandList->OMSetRenderTargets( 1, &rtv, false, &dsv );

    // Set root signature
    m_commandList->SetGraphicsRootSignature( m_rootSignature.Get() );

    // Descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] = 
    { 
        ResourceManager::it()->getSrvCbvUavDescriptorHeap()->getHeap().Get(),
        ResourceManager::it()->getSamplerDescriptorHeap()->getHeap().Get(),
    };
    m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );
    m_commandList->SetGraphicsRootDescriptorTable( 2, ResourceManager::it()->getSrvCbvUavDescriptorHeap()->getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 3, ResourceManager::it()->getSrvCbvUavDescriptorHeap()->getHeap()->GetGPUDescriptorHandleForHeapStart() );
    m_commandList->SetGraphicsRootDescriptorTable( 4, ResourceManager::it()->getSrvCbvUavDescriptorHeap()->getHeap()->GetGPUDescriptorHandleForHeapStart() );

    // Transition backbuffer to render target
    CD3DX12_RESOURCE_BARRIER presentToRenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition
    (
        m_backBuffers[m_currentBackBufferIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_commandList->ResourceBarrier( 1, &presentToRenderTargetBarrier );

    // Clear the backbuffer and depth buffer
    static FLOAT clearColor[ 4 ] = { 0.4f, 0.6f, 0.9f, 1.0f };
    m_commandList->ClearRenderTargetView( rtv, clearColor, 0, nullptr );
    m_commandList->ClearDepthStencilView( dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0.0f, 0, nullptr );

    RenderContext context;
    context.m_commandList = m_commandList;
    context.m_pipelineState.m_rootSignature = m_rootSignature.Get();
    context.m_pipelineState.m_rtvFormats = { { DXGI_FORMAT_R8G8B8A8_UNORM }, 1 };
    context.m_pipelineState.m_dsvFormat = DXGI_FORMAT_D32_FLOAT;
    CD3DX12_RASTERIZER_DESC rasterizerDesc( CD3DX12_DEFAULT{} );
    rasterizerDesc.FrontCounterClockwise = true;
    context.m_pipelineState.m_rasterizer = rasterizerDesc;
    scene.draw( context );

    // Transition backbuffer to present
    CD3DX12_RESOURCE_BARRIER renderTargetToPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition
    (
        m_backBuffers[ m_currentBackBufferIndex ].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    m_commandList->ResourceBarrier( 1, &renderTargetToPresentBarrier );

    // Execute command list
    m_commandList->Close();
    ID3D12CommandList* const commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists( 1, commandLists );

    // Insert fence in command queue
    uint64_t fenceValue = m_fence->signal( m_commandQueue );

    // Present swapchain
    m_swapChain->Present( 1, 0 );
    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Wait for fence
    m_fence->waitForValue( fenceValue );
}
