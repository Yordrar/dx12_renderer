// Windows
#include <Windows.h>
#include <Windowsx.h>
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX 12
#include <d3d12.h>
#include <d3dx12/d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#include <string>

#include <Utils.h>
#include <Window.h>
#include <Renderer.h>
#include <RenderPass.h>
#include <ComputePass.h>
#include <Scene.h>
#include <geometry/Mesh.h>
#include <resource/ResourceManager.h>
#include <geometry/MaterialManager.h>

#include <imgui/imgui.h>



std::unique_ptr<Scene> scene = nullptr;
bool mouse_clicked = false;
int previous_pos_x = -1;
int previous_pos_y = -1;
bool useIndexedVertexBuffer = true;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nShowCmd)
{
    Window window(hInstance,
        hPrevInstance,
        pCmdLine,
        nShowCmd,
        "DX12 Renderer Demo App",
        1280,
        720);

    window.onLeftMouseButtonDown([&](WPARAM wParam, LPARAM lParam)
        {
            mouse_clicked = true;
            previous_pos_x = GET_X_LPARAM(lParam);
            previous_pos_y = GET_Y_LPARAM(lParam);
        });
    window.onLeftMouseButtonUp([&](WPARAM wParam, LPARAM lParam)
        {
            mouse_clicked = false;
            previous_pos_x = -1;
            previous_pos_y = -1;
        });
    window.onMouseMove([&](WPARAM wParam, LPARAM lParam)
        {
            if (mouse_clicked)
            {
                int pos_x = GET_X_LPARAM(lParam);
                int pos_y = GET_Y_LPARAM(lParam);

                scene->getCamera().rotate(-(pos_y - previous_pos_y) / 3.5f, 0);
                scene->getCamera().rotate(0, (pos_x - previous_pos_x) / 3.5f);

                previous_pos_x = pos_x;
                previous_pos_y = pos_y;
            }
        });
    window.onMouseWheel([&](WPARAM wParam, LPARAM lParam)
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            scene->getCamera().m_cameraData.m_position.m128_f32[0] *= 1.0f - (delta / 500.0f);
            scene->getCamera().m_cameraData.m_position.m128_f32[1] *= 1.0f - (delta / 500.0f);
            scene->getCamera().m_cameraData.m_position.m128_f32[2] *= 1.0f - (delta / 500.0f);

            scene->getCamera().move(0.0f, 0.0f, 0.0f);
        });

    Renderer* renderer = window.getRenderer();

    scene = std::make_unique<Scene>(L"mainScene");

    Mesh mesh(L"resource/sponza/sponza.obj", L"shader/test.hlsl");
    mesh.setFrontCounterClockwise(true);
    scene->addMesh(&mesh);

    ResourceHandle mainRenderTarget = ResourceManager::it().createResource( L"mainRenderTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET));
    ResourceManager::it().destroyResource(mainRenderTarget);
    mainRenderTarget = ResourceManager::it().createResource( L"mainRenderTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ) );
    ResourceHandle mainDepthStencilTarget = ResourceManager::it().createResource( L"mainDepthStencilTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE ) );

    RenderPass depthPass( L"Depth Prepass", L"depth", mainRenderTarget.getDefaultRenderTargetView(), mainDepthStencilTarget.getDefaultDepthStencilView() );
    RenderPass mainPass( L"Main Pass", L"main", Descriptor(), mainDepthStencilTarget.getDefaultDepthStencilView(D3D12_DSV_FLAG_READ_ONLY_DEPTH));

    depthPass.setClearRenderTargetsBeforeRendering(true);

    mainPass.setDepthWriteMask(D3D12_DEPTH_WRITE_MASK_ZERO);
    mainPass.setDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);

    ResourceManager::it().createSampler( L"globalSampler" );

    bool show_window = true;
    renderer->registerImguiCallback( [&show_window, &renderer, &depthPass, &mainPass] ()
                                    {
                                        if ( show_window )
                                        {
                                            ImGui::Begin( "Stats", &show_window );
                                            ImGui::LabelText( "", "CPU Time: %.2f ms", renderer->getCPUFrameTime() );
                                            ImGui::LabelText( "", "GPU Time: %.2f ms", renderer->getGPUFrameTime() );
                                            ImGui::LabelText( "", "Depth Prepass Time: %.2f ms", depthPass.getExecutionTimeMilliseconds() );
                                            ImGui::LabelText( "", "Main Pass Time: %.2f ms", mainPass.getExecutionTimeMilliseconds() );
                                            ImGui::End();
                                        }
                                    } );

    
    ResourceHandle testTexture = ResourceManager::it().loadTextureFromFile( L"computeTestTexture", L"resource/demoTex.jpeg" );

    int width = (int)ResourceManager::it().getResourceDesc(testTexture).Width;
    int height = (int)ResourceManager::it().getResourceDesc(testTexture).Height;
    ComputePass mipMapGeneratorPass( L"mipmap_generator", L"shader/testCompute.hlsl", width / 16, height / 16, 1 );

    mipMapGeneratorPass.addResourceView( testTexture.getShaderResourceView(0) );
    mipMapGeneratorPass.addResourceView( testTexture.getUnorderedAccessView(1) );

    Fence testFence(L"TestFence");

    mipMapGeneratorPass.addFenceToSignal(testFence);
    depthPass.addFenceToWaitOn(testFence);

    while ( !window.shouldCloseWindow() )
    {
        renderer->beginFrame();

        mipMapGeneratorPass.setResourceView(0, testTexture.getShaderResourceView(0));
        mipMapGeneratorPass.setResourceView(1, testTexture.getUnorderedAccessView(1));
        mipMapGeneratorPass.setThreadGroupCounts(width / 16, height / 16, 1);
        renderer->submitComputePass(mipMapGeneratorPass);

        mipMapGeneratorPass.setResourceView(0, testTexture.getShaderResourceView(1));
        mipMapGeneratorPass.setResourceView(1, testTexture.getUnorderedAccessView(2));
        mipMapGeneratorPass.setThreadGroupCounts(width / 32, height / 32, 1);
        renderer->submitComputePass(mipMapGeneratorPass);

        renderer->submitRenderPass( depthPass, *scene, { &scene->getCamera() } );
        renderer->submitRenderPass( mainPass, *scene, { &scene->getCamera() } );
        renderer->submitImGui();

        renderer->endFrame();
    }

    renderer->waitForIdleGPU();

    return 0;
}