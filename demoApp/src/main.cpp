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
#include <geometry/FullscreenTriangle.h>
#include <resource/ResourceManager.h>

#include <imgui/imgui.h>



std::unique_ptr<Scene> mainScene = nullptr;
std::unique_ptr<Scene> copyToBackbufferScene = nullptr;
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

                mainScene->getCamera().rotate(-(pos_y - previous_pos_y) / 3.5f, 0);
                mainScene->getCamera().rotate(0, (pos_x - previous_pos_x) / 3.5f);

                previous_pos_x = pos_x;
                previous_pos_y = pos_y;
            }
        });
    window.onMouseWheel([&](WPARAM wParam, LPARAM lParam)
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            mainScene->getCamera().m_cameraData.m_position.m128_f32[0] *= 1.0f - (delta / 500.0f);
            mainScene->getCamera().m_cameraData.m_position.m128_f32[1] *= 1.0f - (delta / 500.0f);
            mainScene->getCamera().m_cameraData.m_position.m128_f32[2] *= 1.0f - (delta / 500.0f);

            mainScene->getCamera().move(0.0f, 0.0f, 0.0f);
        });

    Renderer* renderer = window.getRenderer();

    mainScene = std::make_unique<Scene>(L"mainScene");
    copyToBackbufferScene = std::make_unique<Scene>(L"copyToBackbufferScene");

    Mesh* mesh = new Mesh(L"resource/sponza/sponza.obj", L"shader/test.hlsl");
    mesh->setFrontCounterClockwise(true);
    mainScene->addMesh(mesh);

    FullscreenTriangle* fst = new FullscreenTriangle(L"shader/test.hlsl");
    copyToBackbufferScene->addMesh(fst);

    ResourceHandle mainRenderTarget = ResourceManager::it().createResource( L"mainRenderTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET));
    ResourceManager::it().destroyResource(mainRenderTarget);
    mainRenderTarget = ResourceManager::it().createResource( L"mainRenderTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ) );
    
    fst->addResourceView(mainRenderTarget.getDefaultShaderResourceView());
    
    ResourceHandle mainDepthStencilTarget = ResourceManager::it().createResource( L"mainDepthStencilTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE ) );

    RenderPass depthPass( L"Depth Prepass", L"depth", mainRenderTarget.getDefaultRenderTargetView(), mainDepthStencilTarget.getDefaultDepthStencilView() );
    RenderPass mainPass( L"Main Pass", L"main", mainRenderTarget.getDefaultRenderTargetView(), mainDepthStencilTarget.getDefaultDepthStencilView(D3D12_DSV_FLAG_READ_ONLY_DEPTH));
    RenderPass copyToBackbufferPass( L"Copy to Backbuffer", L"copyToBackbuffer", Descriptor(), mainDepthStencilTarget.getDefaultDepthStencilView(D3D12_DSV_FLAG_READ_ONLY_DEPTH));

    depthPass.setClearRenderTargetsBeforeRendering(true);

    mainPass.setDepthWriteMask(D3D12_DEPTH_WRITE_MASK_ZERO);
    mainPass.setDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);

    copyToBackbufferPass.setDepthWriteMask(D3D12_DEPTH_WRITE_MASK_ZERO);

    D3D12_SAMPLER_DESC samplerDesc = getDefaultSamplerDesc();
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    ResourceManager::it().createSampler( getDefaultSamplerDesc() );
    ResourceManager::it().createSampler( samplerDesc );

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

    while ( !window.shouldCloseWindow() )
    {
        renderer->beginFrame();

        renderer->submitRenderPass( depthPass, *mainScene, { &mainScene->getCamera() } );
        renderer->submitRenderPass( mainPass, *mainScene, { &mainScene->getCamera() } );
        renderer->submitRenderPass( copyToBackbufferPass, *copyToBackbufferScene, { &copyToBackbufferScene->getCamera() } );
        renderer->submitImGui();

        renderer->endFrame();
    }

    renderer->waitForIdleGPU();

    return 0;
}