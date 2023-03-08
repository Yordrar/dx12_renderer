// Windows
#include <Windows.h>
#include <Windowsx.h>

// DirectX 12
#include <DirectXMath.h>

#include <iostream>

#include <Utils.h>
#include <Renderer.h>
#include <Scene.h>
#include <geometry/Mesh.h>
#include <resource/ResourceManager.h>

#define TINYOBJLOADER_IMPLEMENTATION 
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    //AdjustWindowRect( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );

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

    std::string inputfile = "resource/sponza/sponza.obj";
    std::string mtlDir = "resource/sponza/";
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, inputfile.c_str(), mtlDir.c_str(), true );

    if ( !warn.empty() )
    {
        OutputDebugStringA( warn.c_str() );
    }

    if ( !err.empty() )
    {
        OutputDebugStringA( err.c_str() );
    }

    if ( !ret )
    {
        exit( 1 );
    }

    scene = std::make_unique<Scene>(L"mainScene");

    std::vector<Material> loadedMaterials;
    loadedMaterials.reserve( materials.size() );
    for ( tinyobj::material_t const& material : materials )
    {
        int width, height, nrChannelsInFile;
        uint8_t* data = stbi_load( ( mtlDir + material.diffuse_texname ).c_str(), &width, &height, &nrChannelsInFile, 4 );
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1, 1, 0 );

        D3D12_SUBRESOURCE_DATA subresData;
        subresData.pData = data;
        subresData.RowPitch = width * 4;
        subresData.SlicePitch = 0;

        Resource* texture = ResourceManager::it().createResource( StrToWideStr( material.diffuse_texname ).c_str(),
                                                                  resourceDesc,
                                                                  subresData );

        DXGI_FORMAT rtformats[ 8 ] = { DXGI_FORMAT_UNKNOWN };
        rtformats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
        Material::Technique depthTechnique =
        {
            .m_name = L"depth",
            .m_shaderFilename = L"shader/test.hlsl",
            .m_rtFormats = CD3DX12_RT_FORMAT_ARRAY{ rtformats, 8 },
            .m_dsFormat = DXGI_FORMAT_D32_FLOAT
        };
        depthTechnique.m_rasterizerState.FrontCounterClockwise = true;
        Material::Technique mainTechnique =
        {
            .m_name = L"main",
            .m_shaderFilename = L"shader/test.hlsl",
            .m_rtFormats = CD3DX12_RT_FORMAT_ARRAY{ rtformats, 8 },
            .m_dsFormat = DXGI_FORMAT_D32_FLOAT
        };
        mainTechnique.m_rasterizerState.FrontCounterClockwise = true;

        Material::MaterialDesc newMaterialDesc;
        newMaterialDesc.m_name = StrToWideStr( material.name );
        newMaterialDesc.m_techniques.push_back( depthTechnique );
        newMaterialDesc.m_techniques.push_back( mainTechnique );
        if ( texture )
        {
            newMaterialDesc.m_resourceViews.push_back( *texture->getShaderResourceView() );
        }
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );

        loadedMaterials.emplace_back( newMaterialDesc );
    }

    // Load shapes
    for ( size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++ )
    {
        std::vector<Vertex> vertexBuffer;

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for ( size_t faceIdx = 0; faceIdx < shapes[ shapeIdx ].mesh.num_face_vertices.size(); faceIdx++ )
        {
            size_t numFaceVertices = size_t( shapes[ shapeIdx ].mesh.num_face_vertices[ faceIdx ] );

            // Loop over vertices in the face.
            for ( size_t vertexIdx = 0; vertexIdx < numFaceVertices; vertexIdx++ )
            {
                Vertex vertex;
                // access to vertex
                tinyobj::index_t idx = shapes[ shapeIdx ].mesh.indices[ index_offset + vertexIdx ];

                vertex.m_position.x = attrib.vertices[ 3 * size_t( idx.vertex_index ) + 0 ];
                vertex.m_position.y = attrib.vertices[ 3 * size_t( idx.vertex_index ) + 1 ];
                vertex.m_position.z = attrib.vertices[ 3 * size_t( idx.vertex_index ) + 2 ];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if ( idx.normal_index >= 0 )
                {
                    vertex.m_normal.x = attrib.normals[ 3 * size_t( idx.normal_index ) + 0 ];
                    vertex.m_normal.y = attrib.normals[ 3 * size_t( idx.normal_index ) + 1 ];
                    vertex.m_normal.z = attrib.normals[ 3 * size_t( idx.normal_index ) + 2 ];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if ( idx.texcoord_index >= 0 )
                {
                    vertex.m_uvs.x = attrib.texcoords[ 2 * size_t( idx.texcoord_index ) + 0 ];
                    vertex.m_uvs.y = attrib.texcoords[ 2 * size_t( idx.texcoord_index ) + 1 ];
                }

                if ( idx.normal_index >= 0 && idx.texcoord_index >= 0 )
                {
                    // Calculate tangent space
                }

                vertexBuffer.push_back( vertex );
            }
            index_offset += numFaceVertices;
        }

        Mesh* mesh = new Mesh( StrToWideStr( shapes[ shapeIdx ].name ).c_str(), { L"depth", L"main" }, loadedMaterials[ shapes[ shapeIdx ].mesh.material_ids[ 0 ] ] );
        mesh->setVertexBuffer( vertexBuffer.data(), sizeof( Vertex ), static_cast<UINT>( vertexBuffer.size() ) );

        scene->addGeometry( mesh );
    }

    ResourceManager::it().createResource( L"mainRenderTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, windowWidth, windowHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ) );
    ResourceManager::it().createResource( L"mainDepthStencilTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ) );

    RenderPass depthPass(L"Depth Prepass", L"depth", L"", L"mainDepthStencilTarget");
    depthPass.addScene( scene.get() );
    RenderPass mainPass(L"Main Pass", L"main", L"backbuffer", L"mainDepthStencilTarget");
    mainPass.addScene( scene.get() );

    ResourceManager::it().createSampler(L"globalSampler");

    MSG msg = {};
    while ( msg.message != WM_QUIT )
    {
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        renderer.beginFrame();
        renderer.submitPass( depthPass );
        renderer.submitPass( mainPass );
        renderer.endFrame();
    }

    renderer.waitForIdleGPU();

    return 0;
}