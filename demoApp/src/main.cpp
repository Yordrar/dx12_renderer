// Windows
#include <Windows.h>
#include <Windowsx.h>

// DirectX 12
#include <d3dx12/d3dx12.h>
#include <DirectXMath.h>

#include <string>

#include <Utils.h>
#include <Renderer.h>
#include <Scene.h>
#include <geometry/Mesh.h>
#include <resource/ResourceManager.h>
#include <geometry/MaterialManager.h>

#define TINYOBJLOADER_IMPLEMENTATION 
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <meshoptimizer.h>

#include <imgui.h>

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
bool useIndexedVertexBuffer = true;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if ( ImGui_ImplWin32_WndProcHandler( hwnd, message, wParam, lParam ) )
    {
        return DefWindowProcW( hwnd, message, wParam, lParam );
    }

    if ( ImGui::GetCurrentContext() )
    {
        ImGuiIO& io = ImGui::GetIO();
        if ( io.WantCaptureMouse )
        {
            return DefWindowProcW( hwnd, message, wParam, lParam );
        }
    }

    switch ( message )
    {
        case WM_PAINT:
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

                scene->getCamera().rotate( -( pos_y - previous_pos_y ) / 3.5f, 0 );
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
            break;
    }

    return DefWindowProcW( hwnd, message, wParam, lParam );
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
    static auto windowClassName = L"DemoAppWindowClass";

    WNDCLASSEX windowClass =
    {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = &WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hIcon = NULL,
        .hCursor = LoadCursor( NULL, IDC_ARROW ),
        .hbrBackground = (HBRUSH)COLOR_WINDOW,
        .lpszMenuName = NULL,
        .lpszClassName = windowClassName,
        .hIconSm = NULL,
    };

    static ATOM atom = RegisterClassEx( &windowClass );
    assert( atom > 0 );

    int screenWidth = GetSystemMetrics( SM_CXSCREEN );
    int screenHeight = GetSystemMetrics( SM_CYSCREEN );

    LONG desiredClientWidth = 1280;
    LONG desiredClientHeight = 720;
    RECT clientRect = { 0, 0, desiredClientWidth, desiredClientHeight };
    RECT windowRect = { 0, 0, desiredClientWidth, desiredClientHeight };
    AdjustWindowRect( &windowRect, WS_OVERLAPPEDWINDOW, false );

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Center the window within the screen. Clamp to 0, 0 for the top-left corner.
    int windowX = std::max<int>( 0, ( screenWidth - windowWidth ) / 2 );
    int windowY = std::max<int>( 0, ( screenHeight - windowHeight ) / 2 );

    HWND hWnd = CreateWindowEx(
        NULL,
        windowClassName,
        L"DX12 Renderer Demo",
        WS_OVERLAPPEDWINDOW,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    assert( hWnd );
    ShowWindow( hWnd, nCmdShow );

    Renderer renderer( hWnd, clientRect );

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

    std::vector<std::wstring> loadedMaterials;
    loadedMaterials.reserve( materials.size() );
    for ( tinyobj::material_t const& material : materials )
    {
        int width, height, nrChannelsInFile;
        uint8_t* data = stbi_load( ( mtlDir + material.diffuse_texname ).c_str(), &width, &height, &nrChannelsInFile, 4 );
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1 );

        D3D12_SUBRESOURCE_DATA subresData;
        subresData.pData = data;
        subresData.RowPitch = width * 4;
        subresData.SlicePitch = 0;

        ResourceHandle texture = ResourceManager::it().createResource( StrToWideStr( material.diffuse_texname ).c_str(),
                                                                  resourceDesc,
                                                                  subresData );
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
        {
            .Format = resourceDesc.Format,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        };
        srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0;

        DXGI_FORMAT rtformats[ 8 ] = { DXGI_FORMAT_UNKNOWN };
        rtformats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
        Material::Technique depthTechnique =
        {
            .m_name = L"depth",
            .m_vertexShaderFilename = L"shader/test.hlsl",
            .m_pixelShaderFilename = L"",
            .m_rtFormats = CD3DX12_RT_FORMAT_ARRAY{ rtformats, 1 },
            .m_dsFormat = DXGI_FORMAT_D32_FLOAT
        };
        depthTechnique.m_rasterizerState.FrontCounterClockwise = true;
        Material::Technique mainTechnique =
        {
            .m_name = L"main",
            .m_vertexShaderFilename = L"shader/test.hlsl",
            .m_pixelShaderFilename = L"shader/test.hlsl",
            .m_rtFormats = CD3DX12_RT_FORMAT_ARRAY{ rtformats, 1 },
            .m_dsFormat = DXGI_FORMAT_D32_FLOAT,
        };
        mainTechnique.m_rasterizerState.FrontCounterClockwise = true;
        mainTechnique.m_depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        mainTechnique.m_depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

        Material::MaterialDesc newMaterialDesc;
        newMaterialDesc.m_name = StrToWideStr( material.name );
        newMaterialDesc.m_techniques.push_back( depthTechnique );
        newMaterialDesc.m_techniques.push_back( mainTechnique );
        if ( texture.isValid() )
        {
            newMaterialDesc.m_resourceViews.push_back(ResourceManager::it().getShaderResourceView( texture, srvDesc ) );
        }
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
        newMaterialDesc.m_inputLayout.push_back( D3D12_INPUT_ELEMENT_DESC{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );

        MaterialManager::it().createMaterial( newMaterialDesc );
        loadedMaterials.push_back( newMaterialDesc.m_name );
    }

    // Load shapes
    for ( size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++ )
    {
        std::vector<Vertex> unindexedVertexBuffer;
        Mesh::AABB currentMeshAABB;

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

                DirectX::XMVECTOR positionVector = DirectX::XMVectorSet( vertex.m_position.x, vertex.m_position.y, vertex.m_position.z, 0.0f );
                currentMeshAABB.m_minBounds = DirectX::XMVectorMin( currentMeshAABB.m_minBounds, positionVector ); 
                currentMeshAABB.m_maxBounds = DirectX::XMVectorMax( currentMeshAABB.m_maxBounds, positionVector );

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

                unindexedVertexBuffer.push_back( vertex );
            }
            index_offset += numFaceVertices;
        }

        size_t index_count = shapes[ shapeIdx ].mesh.num_face_vertices.size() * 3;
        std::vector<UINT> indexBuffer( index_count );

        std::vector<unsigned int> remap( index_count );
        size_t vertex_count = meshopt_generateVertexRemap( &remap[ 0 ], NULL, index_count, &unindexedVertexBuffer[ 0 ], index_count, sizeof( Vertex ) );
        std::vector<Vertex> vertexBuffer( vertex_count );

        meshopt_remapIndexBuffer( indexBuffer.data(), NULL, index_count, &remap[ 0 ] );
        meshopt_remapVertexBuffer( vertexBuffer.data(), &unindexedVertexBuffer[ 0 ], index_count, sizeof( Vertex ), &remap[ 0 ] );
        meshopt_optimizeVertexCache( indexBuffer.data(), indexBuffer.data(), index_count, vertex_count );
        meshopt_optimizeOverdraw( indexBuffer.data(), indexBuffer.data(), index_count, &vertexBuffer[ 0 ].m_position.x, vertex_count, sizeof( Vertex ), 1.05f );
        meshopt_optimizeVertexFetch( vertexBuffer.data(), indexBuffer.data(), index_count, vertexBuffer.data(), vertex_count, sizeof( Vertex ) );

        Mesh* mesh = new Mesh( StrToWideStr( shapes[ shapeIdx ].name ).c_str(), loadedMaterials[ shapes[ shapeIdx ].mesh.material_ids[ 0 ] ].c_str() );
        if ( useIndexedVertexBuffer )
        {
            mesh->setVertexBuffer( vertexBuffer.data(), sizeof( Vertex ), static_cast<UINT>( vertexBuffer.size() ) );
            mesh->setIndexBuffer( indexBuffer.data(), static_cast<UINT>( index_count ) );
        }
        else
        {
            mesh->setVertexBuffer( unindexedVertexBuffer.data(), sizeof( Vertex ), static_cast<UINT>( unindexedVertexBuffer.size() ) );
        }
        mesh->setAABB( currentMeshAABB );

        scene->addMesh( mesh );
    }

    ResourceHandle mainRenderTarget = ResourceManager::it().createResource( L"mainRenderTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, desiredClientWidth, desiredClientHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ) );
    ResourceHandle mainDepthStencilTarget = ResourceManager::it().createResource( L"mainDepthStencilTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, desiredClientWidth, desiredClientHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE ) );

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc =
    {
        .Format = ResourceManager::it().getResourceDesc(mainRenderTarget).Format,
        .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
    };
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc =
    {
        .Format = ResourceManager::it().getResourceDesc(mainDepthStencilTarget).Format,
        .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
        .Flags = D3D12_DSV_FLAG_NONE
    };
    dsvDesc.Texture2D.MipSlice = 0;

    RenderPass depthPass( L"Depth Prepass", L"depth", ResourceManager::it().getRenderTargetView( mainRenderTarget, rtvDesc ), ResourceManager::it().getDepthStencilView( mainDepthStencilTarget, dsvDesc ) );
    dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
    RenderPass mainPass( L"Main Pass", L"main", Descriptor(), ResourceManager::it().getDepthStencilView( mainDepthStencilTarget, dsvDesc ) );

    ResourceManager::it().createSampler( L"globalSampler" );

    bool show_window = true;
    renderer.registerImguiCallback( [&show_window, &renderer, &depthPass, &mainPass] ()
                                    {
                                        if ( show_window )
                                        {
                                            ImGui::Begin( "Stats", &show_window );
                                            ImGui::LabelText( "", "CPU Time: %.2f ms", renderer.getCPUFrameTime() );
                                            ImGui::LabelText( "", "GPU Time: %.2f ms", renderer.getGPUFrameTime() );
                                            ImGui::LabelText( "", "Depth Prepass Time: %.2f ms", depthPass.getExecutionTimeMilliseconds() );
                                            ImGui::LabelText( "", "Main Pass Time: %.2f ms", mainPass.getExecutionTimeMilliseconds() );
                                            ImGui::End();
                                        }
                                    } );

    int width, height, nrChannelsInFile;
    uint8_t* data = stbi_load( "resource/demoTex.jpeg", &width, &height, &nrChannelsInFile, 4 );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 3, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS );

    D3D12_SUBRESOURCE_DATA subresData;
    subresData.pData = data;
    subresData.RowPitch = width * 4;
    subresData.SlicePitch = 0;
    ResourceHandle testTexture = ResourceManager::it().createResource( L"computeTestTexture", resourceDesc, subresData );

    ComputePass mipMapGeneratorPass( L"mipmap_generator", L"shader/testCompute.hlsl", width / 16, height / 16, 1 );

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescMip0 =
    {
        .Format = ResourceManager::it().getResourceDesc(testTexture).Format,
        .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    };
    srvDescMip0.Texture2D.MipLevels = 3;
    srvDescMip0.Texture2D.MostDetailedMip = 0;
    srvDescMip0.Texture2D.PlaneSlice = 0;
    srvDescMip0.Texture2D.ResourceMinLODClamp = 0;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescMip1 =
    {
        .Format = ResourceManager::it().getResourceDesc(testTexture).Format,
        .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    };
    srvDescMip1.Texture2D.MipLevels = 2;
    srvDescMip1.Texture2D.MostDetailedMip = 1;
    srvDescMip1.Texture2D.PlaneSlice = 0;
    srvDescMip1.Texture2D.ResourceMinLODClamp = 0;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescMip1 =
    {
        .Format = ResourceManager::it().getResourceDesc(testTexture).Format,
        .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
    };
    uavDescMip1.Texture2D.PlaneSlice = 0;
    uavDescMip1.Texture2D.MipSlice = 1;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescMip2 =
    {
        .Format = ResourceManager::it().getResourceDesc(testTexture).Format,
        .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
    };
    uavDescMip2.Texture2D.PlaneSlice = 0;
    uavDescMip2.Texture2D.MipSlice = 2;

    mipMapGeneratorPass.addResourceView(ResourceManager::it().getShaderResourceView(testTexture, srvDescMip0));
    mipMapGeneratorPass.addResourceView(ResourceManager::it().getUnorderedAccessView(testTexture, uavDescMip1));
    renderer.beginFrame();
    renderer.submitComputePass( mipMapGeneratorPass );
    renderer.endFrame();

    mipMapGeneratorPass.setResourceView( 0, ResourceManager::it().getShaderResourceView(testTexture, srvDescMip1));
    mipMapGeneratorPass.setResourceView( 1, ResourceManager::it().getUnorderedAccessView(testTexture, uavDescMip2));
    mipMapGeneratorPass.setThreadGroupCounts( width / 32, height / 32, 1 );
    renderer.beginFrame();
    renderer.submitComputePass( mipMapGeneratorPass );
    renderer.endFrame();

    MSG msg{};
    while ( msg.message != WM_QUIT )
    {
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        renderer.beginFrame();
        renderer.submitRenderPass( depthPass, *scene, { &scene->getCamera() } );
        renderer.submitRenderPass( mainPass, *scene, { &scene->getCamera() } );
        renderer.endFrame();
    }

    renderer.waitForIdleGPU();

    return 0;
}