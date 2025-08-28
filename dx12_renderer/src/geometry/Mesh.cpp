#include "Mesh.h"

#include <Utils.h>
#include <BarrierRecorder.h>
#include <geometry/MaterialManager.h>
#include <geometry/Material.h>
#include <geometry/ShaderManager.h>
#include <resource/Resource.h>
#include <resource/ResourceManager.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::vector<D3D12_INPUT_ELEMENT_DESC> Mesh::s_inputLayout;

Mesh::Mesh( wchar_t const* objFilepath, wchar_t const* shaderFilepath )
{
    std::string inputFile(WideStrToStr(objFilepath));
    std::string mtlDir = std::filesystem::path(inputFile).remove_filename().string();
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warning;
    std::string error;

    bool loadedSuccessfully = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, inputFile.c_str(), mtlDir.c_str(), true);

#if defined(RENDERER_DEBUG)
    if (!warning.empty())
    {
        OutputDebugStringA(warning.c_str());
    }

    if (!error.empty())
    {
        OutputDebugStringA(error.c_str());
    }
#endif

    if (!loadedSuccessfully)
    {
        exit(1);
    }

    std::vector<std::pair<std::wstring, Material*>> loadedMaterials;
    loadedMaterials.reserve(materials.size());
    for (tinyobj::material_t const& material : materials)
    {
        int width, height, nrChannelsInFile;
        uint8_t* data = stbi_load((mtlDir + material.diffuse_texname).c_str(), &width, &height, &nrChannelsInFile, 4);
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);

        D3D12_SUBRESOURCE_DATA subresData;
        subresData.pData = data;
        subresData.RowPitch = width * 4;
        subresData.SlicePitch = 0;

        ResourceHandle texture = ResourceManager::it().createResource(StrToWideStr(material.diffuse_texname).c_str(),
            resourceDesc,
            subresData);

        Material::MaterialDesc newMaterialDesc;
        newMaterialDesc.m_name = StrToWideStr(material.name);
        if (texture.isValid())
        {
            newMaterialDesc.m_resourceViews.push_back(texture.getDefaultShaderResourceView());
        }

        loadedMaterials.push_back(std::pair(newMaterialDesc.m_name, new Material(newMaterialDesc)));
    }

    // Load shapes
    for (size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++)
    {
        std::vector<Vertex> unindexedVertexBuffer;

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t faceIdx = 0; faceIdx < shapes[shapeIdx].mesh.num_face_vertices.size(); faceIdx++)
        {
            size_t numFaceVertices = size_t(shapes[shapeIdx].mesh.num_face_vertices[faceIdx]);

            // Loop over vertices in the face.
            for (size_t vertexIdx = 0; vertexIdx < numFaceVertices; vertexIdx++)
            {
                Vertex vertex;
                // access to vertex
                tinyobj::index_t idx = shapes[shapeIdx].mesh.indices[index_offset + vertexIdx];

                vertex.m_position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                vertex.m_position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                vertex.m_position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                DirectX::XMVECTOR positionVector = DirectX::XMVectorSet(vertex.m_position.x, vertex.m_position.y, vertex.m_position.z, 0.0f);

                // Check if normal_index is zero or positive. negative = no normal data
                if (idx.normal_index >= 0)
                {
                    vertex.m_normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    vertex.m_normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    vertex.m_normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // Check if texcoord_index is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0)
                {
                    vertex.m_uvs.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    vertex.m_uvs.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                // If we have normals and UVs, we can calculate the tangent space for the vertex
                if (idx.normal_index >= 0 && idx.texcoord_index >= 0)
                {
                    // TODO Calculate tangent space
                }

                unindexedVertexBuffer.push_back(vertex);
            }
            index_offset += numFaceVertices;
        }

        size_t index_count = shapes[shapeIdx].mesh.num_face_vertices.size() * 3;
        std::vector<UINT> indexBuffer(index_count);

        std::vector<unsigned int> remap(index_count);
        size_t vertex_count = meshopt_generateVertexRemap(&remap[0], NULL, index_count, &unindexedVertexBuffer[0], index_count, sizeof(Vertex));
        std::vector<Vertex> vertexBuffer(vertex_count);

        meshopt_remapIndexBuffer(indexBuffer.data(), NULL, index_count, &remap[0]);
        meshopt_remapVertexBuffer(vertexBuffer.data(), &unindexedVertexBuffer[0], index_count, sizeof(Vertex), &remap[0]);
        meshopt_optimizeVertexCache(indexBuffer.data(), indexBuffer.data(), index_count, vertex_count);
        meshopt_optimizeOverdraw(indexBuffer.data(), indexBuffer.data(), index_count, &vertexBuffer[0].m_position.x, vertex_count, sizeof(Vertex), 1.05f);
        meshopt_optimizeVertexFetch(vertexBuffer.data(), indexBuffer.data(), index_count, vertexBuffer.data(), vertex_count, sizeof(Vertex));

        m_submeshes.emplace_back();
        Submesh& newSubmesh = m_submeshes.back();
        newSubmesh.m_name = shapes[shapeIdx].name;
        newSubmesh.m_vertexBuffer = std::make_unique<VertexBuffer>(vertexBuffer.data(), sizeof(Vertex), static_cast<UINT>(vertexBuffer.size()));
        newSubmesh.m_indexBuffer = std::make_unique<IndexBuffer>(indexBuffer.data(), static_cast<UINT>(index_count));
        newSubmesh.m_shaderFilepath = shaderFilepath;
        newSubmesh.m_materialName = loadedMaterials[shapes[shapeIdx].mesh.material_ids[0]].first;
        newSubmesh.m_material.reset(loadedMaterials[shapes[shapeIdx].mesh.material_ids[0]].second);
    }
}

void Mesh::record(ComPtr<ID3D12GraphicsCommandList> commandList) const
{
    for (Submesh const& submesh : m_submeshes)
    {
        submesh.record(commandList);
    }
}

void Mesh::setFrontCounterClockwise(bool frontCounterClockwise)
{
    for (Submesh& submesh : m_submeshes)
    {
        submesh.m_rasterizerState.FrontCounterClockwise = frontCounterClockwise;
    }
}

void Mesh::Submesh::record( ComPtr<ID3D12GraphicsCommandList> commandList ) const
{
    BarrierRecorder br;
    br.recordBarrierTransition(m_vertexBuffer->getResource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    br.recordBarrierTransition(m_indexBuffer->getResource(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
    br.submitBarriers(commandList);

    commandList->IASetPrimitiveTopology( m_primitiveTopology );
    m_vertexBuffer->bind( commandList );

    if( m_indexBuffer )
    {
        m_indexBuffer->bind(commandList);
        commandList->DrawIndexedInstanced(m_indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }
    else
    {
        commandList->DrawInstanced(static_cast<UINT>( m_vertexBuffer->getVertexCount() ), 1, 0, 0);
    }
}
