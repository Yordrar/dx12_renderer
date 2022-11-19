// Windows headers
#include <Windows.h>
#include <Windowsx.h>
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX 12 headers
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
// D3D12 extension library
#include <d3dx12.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Renderer.h>
#include <Scene.h>
#include <geometry/Mesh.h>
#include <resource/ResourceManager.h>

#include <iostream>

struct Vertex
{
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_normal;
    DirectX::XMFLOAT2 m_uvs;
    DirectX::XMFLOAT3 m_tangent;
    DirectX::XMFLOAT3 m_bitangent;
};

std::unique_ptr<Scene> scene = nullptr;
bool mouse_clicked = false;
int previous_pos_x = -1;
int previous_pos_y = -1;
LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        case WM_PAINT:
            break; 
        break;
        // The default window procedure will play a system notification sound 
        // when pressing the Alt+Enter keyboard combination if this message is 
        // not handled.
        case WM_SYSCHAR:
            break;
        case WM_SIZE:
        break;
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
        case WM_LBUTTONDOWN:
            mouse_clicked = true;
            previous_pos_x = GET_X_LPARAM( lParam );
            previous_pos_y = GET_Y_LPARAM( lParam );
            break;
        case WM_LBUTTONUP:
            mouse_clicked = false;
            previous_pos_x = -1;
            previous_pos_y = -1;
            break;
        case WM_MOUSEMOVE:
            if ( mouse_clicked )
            {
                int pos_x = GET_X_LPARAM( lParam );
                int pos_y = GET_Y_LPARAM( lParam );

                scene->getCamera().rotate( ( pos_y - previous_pos_y ) / 3.5f, 0 );
                scene->getCamera().rotate( 0, ( pos_x - previous_pos_x ) / 3.5f );

                previous_pos_x = pos_x;
                previous_pos_y = pos_y;
            }
            break;
        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM( wParam );
            scene->getCamera().m_cameraData.m_position.m128_f32[ 0 ] *= 1.0f - ( delta / 500.0f );
            scene->getCamera().m_cameraData.m_position.m128_f32[ 1 ] *= 1.0f - ( delta / 500.0f );
            scene->getCamera().m_cameraData.m_position.m128_f32[ 2 ] *= 1.0f - ( delta / 500.0f );

            scene->getCamera().move( 0.0f, 0.0f, 0.0f );
            break;
        }
        default:
            return DefWindowProcW( hwnd, message, wParam, lParam );
    }

    return 0;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
    static auto windowClassName = L"DemoWndClass";

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof( WNDCLASSEX );
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = ::LoadIcon( hInstance, MAKEINTRESOURCE( 32512 ) );
    windowClass.hCursor = ::LoadCursor( NULL, IDC_ARROW );
    windowClass.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = windowClassName;
    windowClass.hIconSm = ::LoadIcon( hInstance, MAKEINTRESOURCE( 32512 ) );

    static ATOM atom = ::RegisterClassExW( &windowClass );
    assert( atom > 0 );

    int screenWidth = ::GetSystemMetrics( SM_CXSCREEN );
    int screenHeight = ::GetSystemMetrics( SM_CYSCREEN );

    RECT windowRect = { 0, 0, static_cast<LONG>( 1280 ), static_cast<LONG>( 720 ) };
    AdjustWindowRect( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Center the window within the screen. Clamp to 0, 0 for the top-left corner.
    int windowX = std::max<int>( 0, ( screenWidth - windowWidth ) / 2 );
    int windowY = std::max<int>( 0, ( screenHeight - windowHeight ) / 2 );

    HWND hWnd = CreateWindowExW(
        NULL,
        windowClassName,
        L"DX12 Renderer Demo",
        WS_OVERLAPPEDWINDOW,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        hInstance,
        nullptr
    );
    assert( hWnd );
    ShowWindow( hWnd, SW_SHOW );

    Renderer renderer( hWnd, windowRect );

    Assimp::Importer importer;
    const aiScene* aiScene = importer.ReadFile( "resource/suzanne.obj",
                                                aiProcess_CalcTangentSpace |
                                                aiProcess_Triangulate |
                                                aiProcess_GenNormals |
                                                aiProcess_ValidateDataStructure |
                                                aiProcess_GenUVCoords |
                                                aiProcess_FixInfacingNormals |
                                                aiProcess_JoinIdenticalVertices |
                                                aiProcess_SortByPType );
    aiMesh* ai_mesh = aiScene->mMeshes[ 0 ];

    UINT vertices_count = ai_mesh->mNumVertices;
    Vertex* vertex_buffer_data = new Vertex[ vertices_count ];
    for ( unsigned int i = 0; i < ai_mesh->mNumVertices; i++ )
    {
        Vertex vert;
        vert.m_position.x = ai_mesh->mVertices[ i ].x;
        vert.m_position.y = ai_mesh->mVertices[ i ].y;
        vert.m_position.z = ai_mesh->mVertices[ i ].z;

        vert.m_normal.x = ai_mesh->mNormals[ i ].x;
        vert.m_normal.y = ai_mesh->mNormals[ i ].y;
        vert.m_normal.z = ai_mesh->mNormals[ i ].z;

        vert.m_uvs.x = ai_mesh->mTextureCoords[ 0 ][ i ].x;
        vert.m_uvs.y = ai_mesh->mTextureCoords[ 0 ][ i ].y;

        vert.m_tangent.x = ai_mesh->mTangents[ i ].x;
        vert.m_tangent.y = ai_mesh->mTangents[ i ].y;
        vert.m_tangent.z = ai_mesh->mTangents[ i ].z;

        vert.m_bitangent.x = ai_mesh->mBitangents[ i ].x;
        vert.m_bitangent.y = ai_mesh->mBitangents[ i ].y;
        vert.m_bitangent.z = ai_mesh->mBitangents[ i ].z;

        vertex_buffer_data[ i ] = vert;
    }

    UINT indices_count = ai_mesh->mNumFaces * 3;
    UINT* vertex_indices_data = new UINT[ indices_count ];
    for ( unsigned int i = 0; i < ai_mesh->mNumFaces; i++ )
    {
        aiFace face = ai_mesh->mFaces[ i ];
        for ( unsigned int j = 0; j < face.mNumIndices; j++ )
            vertex_indices_data[ i * 3 + j ] = face.mIndices[ j ];
    }

    ResourceManager::it().createTexture( "mainRenderTarget", windowWidth, windowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET );
    ResourceManager::it().createTexture( "mainDepthStencilTarget", windowWidth, windowHeight, DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );

    scene = std::make_unique<Scene>( "mainScene" );

    Mesh* mesh = new Mesh( "suzanne", {"depth","main"});
    mesh->addInputLayoutElement( "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT );
    mesh->addInputLayoutElement( "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT );
    mesh->addInputLayoutElement( "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT );
    mesh->addInputLayoutElement( "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT );
    mesh->addInputLayoutElement( "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT );
    mesh->setVertexBuffer( vertex_buffer_data, sizeof(Vertex), vertices_count );
    mesh->setIndexBuffer( vertex_indices_data, indices_count );
    mesh->setShaders( "shader/test_vs.hlsl", "shader/test_ps.hlsl" );
    mesh->addResource( ResourceManager::it().createTexture( "suzanne_demoTex", "resource/demoTex.jpeg", DXGI_FORMAT_R8G8B8A8_UNORM ) );
    scene->addGeometry( mesh );

    RenderPass depthPass( "Depth Prepass", "depth", "", "mainDepthStencilTarget" );
    RenderPass mainPass( "Main Pass", "main", "mainRenderTarget", "mainDepthStencilTarget" );
    RenderPass copyToBackbufferPass( "Copy to Backbuffer", "copy", "backbuffer", "" );
    renderer.addRenderPass( depthPass );
    renderer.addRenderPass( mainPass );
    renderer.addRenderPass( copyToBackbufferPass );

    MSG msg = {};
    while ( msg.message != WM_QUIT )
    {
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        renderer.drawScene(*scene);
    }

    return 0;
}