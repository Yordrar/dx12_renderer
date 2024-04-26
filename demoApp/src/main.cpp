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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <meshoptimizer.h>

#include <imgui/imgui.h>

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
            newMaterialDesc.m_resourceViews.push_back( texture.getDefaultShaderResourceView() );
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

    ResourceHandle mainRenderTarget = ResourceManager::it().createResource( L"mainRenderTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET));
    ResourceManager::it().destroyResource(mainRenderTarget);
    mainRenderTarget = ResourceManager::it().createResource( L"mainRenderTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ) );
    ResourceHandle mainDepthStencilTarget = ResourceManager::it().createResource( L"mainDepthStencilTarget", CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, renderer->getClientWidth(), renderer->getClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE ) );

    RenderPass depthPass( L"Depth Prepass", L"depth", mainRenderTarget.getDefaultRenderTargetView(), mainDepthStencilTarget.getDefaultDepthStencilView() );
    RenderPass mainPass( L"Main Pass", L"main", Descriptor(), mainDepthStencilTarget.getDefaultDepthStencilView(D3D12_DSV_FLAG_READ_ONLY_DEPTH));

    depthPass.setClearRenderTargetsBeforeRendering(true);

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

    int width, height, nrChannelsInFile;
    uint8_t* data = stbi_load( "resource/demoTex.jpeg", &width, &height, &nrChannelsInFile, 4 );
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 3, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS );

    D3D12_SUBRESOURCE_DATA subresData;
    subresData.pData = data;
    subresData.RowPitch = width * 4;
    subresData.SlicePitch = 0;
    ResourceHandle testTexture = ResourceManager::it().createResource( L"computeTestTexture", resourceDesc, subresData );

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