#include "Renderer.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

#include <RendererConstants.h>
#include <resource/ResourceManager.h>
#include <resource/DescriptorHeap.h>

RECT Renderer::s_windowRect;
ComPtr<ID3D12Device2> Renderer::s_device{ nullptr };
UINT Renderer::s_currentBackBufferIndex = 0;
uint64_t Renderer::s_timestampFrequency = 0;
ComPtr<ID3D12RootSignature> Renderer::s_rootSignature{ nullptr };

Renderer::Renderer( HWND hWnd, RECT windowRect )
    : m_hWnd(hWnd)
    , m_swapChain( nullptr )
    , m_frameFence( nullptr )
    , m_graphicsCmdQueue( nullptr )
    , m_computeCmdQueue( nullptr )
    , m_imguiCallbackRegistered( false )
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
    CD3DX12_ROOT_PARAMETER slotRootParameters[ 14 ] = {};

    slotRootParameters[ 0 ].InitAsConstantBufferView( 0, 0 ); // Render Pass buffer
    slotRootParameters[ 1 ].InitAsConstantBufferView( 1, 0 ); // Camera buffer
    slotRootParameters[ 2 ].InitAsConstantBufferView( 2, 0 ); // Material buffer
    slotRootParameters[ 3 ].InitAsConstantBufferView( 3, 0 ); // Geometry buffer

    D3D12_DESCRIPTOR_RANGE srvRangeBuffer =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 4 ].InitAsDescriptorTable( 1, &srvRangeBuffer );

    D3D12_DESCRIPTOR_RANGE srvRangeTexture1D =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 1,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 5 ].InitAsDescriptorTable( 1, &srvRangeTexture1D );

    D3D12_DESCRIPTOR_RANGE srvRangeTexture2D =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 2,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 6 ].InitAsDescriptorTable( 1, &srvRangeTexture2D );

    D3D12_DESCRIPTOR_RANGE srvRangeTexture3D =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 3,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 7 ].InitAsDescriptorTable( 1, &srvRangeTexture3D );

    D3D12_DESCRIPTOR_RANGE srvRangeTextureCube =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 4,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 8 ].InitAsDescriptorTable( 1, &srvRangeTextureCube );



    D3D12_DESCRIPTOR_RANGE srvRangeRWBuffer =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 9 ].InitAsDescriptorTable( 1, &srvRangeRWBuffer );

    D3D12_DESCRIPTOR_RANGE srvRangeRWTexture1D =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 1,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 10 ].InitAsDescriptorTable( 1, &srvRangeRWTexture1D );

    D3D12_DESCRIPTOR_RANGE srvRangeRWTexture2D =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 2,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 11 ].InitAsDescriptorTable( 1, &srvRangeRWTexture2D );

    D3D12_DESCRIPTOR_RANGE srvRangeRWTexture3D =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 3,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 12 ].InitAsDescriptorTable( 1, &srvRangeRWTexture3D );



    D3D12_DESCRIPTOR_RANGE srvRangeSampler =
    {
        .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
        .NumDescriptors = RendererConstants::sc_numDescriptorsInHeaps,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .OffsetInDescriptorsFromTableStart = 0,
    };
    slotRootParameters[ 13 ].InitAsDescriptorTable( 1, &srvRangeSampler );



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
    s_rootSignature->SetName( L"Global Root Signature" );

    m_graphicsCmdQueue->GetTimestampFrequency( &s_timestampFrequency );

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init( hWnd );
    ImGui_ImplDX12_Init( device().Get(),
                         RendererConstants::sc_numBackBuffers,
                         DXGI_FORMAT_R8G8B8A8_UNORM,
                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get(),
                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get()->GetCPUDescriptorHandleForHeapStart(),
                         DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get()->GetGPUDescriptorHandleForHeapStart() );

    for ( int i = 0; i < RendererConstants::sc_numBackBuffers; ++i )
    {
        Renderer::device()->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_imguiCommandAllocators[ i ] ) );
    }

    HRESULT result = Renderer::device()->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_imguiCommandAllocators[ 0 ].Get(), nullptr, IID_PPV_ARGS( &m_imguiCommandList ) );
    m_imguiCommandList->Close();
    std::wstring commandListName = L"imgui_commandList";
    m_imguiCommandList->SetName( commandListName.c_str() );
}

Renderer::~Renderer()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

static auto start = std::chrono::high_resolution_clock::now();
static std::vector<RenderPass*> submittedRenderPasses;
static std::vector<ComputePass*> submittedComputePasses;
void Renderer::beginFrame()
{
    submittedRenderPasses.clear();
    submittedComputePasses.clear();

    m_gpuFrameTime = 0;

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    start = std::chrono::high_resolution_clock::now();
}

void Renderer::submitRenderPass( RenderPass& pass, Scene const& scene, std::vector<Camera*> const& cameras )
{
    pass.record( scene, cameras );
    m_gpuFrameTime += pass.getExecutionTimeMilliseconds();
    submittedRenderPasses.push_back( &pass );
}

void Renderer::submitComputePass( ComputePass& pass )
{
    pass.record();
    m_gpuFrameTime += pass.getExecutionTimeMilliseconds();
    submittedComputePasses.push_back( &pass );
}

void Renderer::endFrame()
{
    recordImguiCommandList();

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( end - start );
    m_cpuFrameTime = static_cast<double>( elapsed.count() );

    for ( RenderPass* pass : submittedRenderPasses )
    {
        pass->waitOnComputePasses( m_graphicsCmdQueue, submittedComputePasses );
        ID3D12CommandList* commandList = pass->getCommandList();
        m_graphicsCmdQueue->ExecuteCommandLists( 1, &commandList );
    }
    ID3D12CommandList* imguiCmdList = m_imguiCommandList.Get();
    m_graphicsCmdQueue->ExecuteCommandLists( 1, &imguiCmdList );

    for ( ComputePass* pass : submittedComputePasses )
    {
        ID3D12CommandList* commandList = pass->getCommandList();
        m_computeCmdQueue->ExecuteCommandLists( 1, &commandList );
        pass->getFence().GPUSignal( m_graphicsCmdQueue, pass->getFence().getCompletedValue() + 1 );
    }

    m_swapChain->Present( 1, 0 );

    // Schedule a Signal command in the queue.
    uint64_t const currentFenceValue = m_fenceValues[ s_currentBackBufferIndex ];
    m_frameFence->GPUSignal( m_graphicsCmdQueue, currentFenceValue );

    // Update the frame index.
    s_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    m_frameFence->CPUWait( m_fenceValues[ s_currentBackBufferIndex ] );

    // Set the fence value for the next frame.
    m_fenceValues[ s_currentBackBufferIndex ] = currentFenceValue + 1;
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

void Renderer::recordImguiCommandList()
{
    if ( m_imguiCallbackRegistered )
    {
        m_imguiUserCallback();
    }
    ImGui::Render();

    ComPtr<ID3D12CommandAllocator> currentCommandAllocator = m_imguiCommandAllocators[ getCurrentBackbufferIndex() ];
    currentCommandAllocator->Reset();
    m_imguiCommandList->Reset( currentCommandAllocator.Get(), nullptr );

    PIXBeginEvent( m_imguiCommandList.Get(), PIX_COLOR_DEFAULT, "Imgui" );

    ID3D12DescriptorHeap* descriptorHeaps[] =
    {
        DescriptorHeap::getDescriptorHeapCbvSrvUav().getHeap().Get(),
    };
    m_imguiCommandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

    Resource* renderTarget = ResourceManager::it().getCurrentBackbufferResource();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = renderTarget->getRenderTargetView()->getView();
    m_imguiCommandList->OMSetRenderTargets( 1, &rtv, false, nullptr );

    ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), m_imguiCommandList.Get() );

    CD3DX12_RESOURCE_BARRIER renderTargetBarrier = renderTarget->getTransitionBarrier( D3D12_RESOURCE_STATE_PRESENT );
    m_imguiCommandList->ResourceBarrier( 1, &renderTargetBarrier );

    PIXEndEvent();

    m_imguiCommandList->Close();
}
