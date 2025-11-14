#include "MeshImporter.h"

#include <Utils.h>
#include <geometry/Mesh.h>
#include <geometry/VertexBuffer.h>
#include <geometry/IndexBuffer.h>
#include <geometry/Material.h>
#include <geometry/ShaderManager.h>
#include <resource/Resource.h>
#include <resource/ResourceManager.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Mesh* MeshImporter::createEmpty()
{
    return new Mesh();
}

ResourceHandle helperLoadTextureFromFile(std::string const& filePath, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
{
    int width, height, nrChannelsInFile;
    uint8_t* data = stbi_load(filePath.c_str(), &width, &height, &nrChannelsInFile, 4);
    if (!data)
    {
        return ResourceHandle();
    }

    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1);

    D3D12_SUBRESOURCE_DATA subresData;
    subresData.pData = data;
    subresData.RowPitch = width * 4;
    subresData.SlicePitch = 0;

    return ResourceManager::it().createResource(StrToWideStr(filePath).c_str(), resourceDesc, subresData);
}

Mesh* MeshImporter::createFromObjFile(wchar_t const* objFilepath, wchar_t const* shaderFilepath)
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

    std::vector< std::shared_ptr<Material> > loadedMaterials;
    loadedMaterials.reserve(materials.size());
    for (tinyobj::material_t const& material : materials)
    {
        ResourceHandle diffuseTexture = helperLoadTextureFromFile(mtlDir + material.diffuse_texname);
        ResourceHandle normalTexture = helperLoadTextureFromFile(mtlDir + material.normal_texname);
        ResourceHandle roughnessTexture = helperLoadTextureFromFile(mtlDir + material.roughness_texname);
        ResourceHandle metallicTexture = helperLoadTextureFromFile(mtlDir + material.metallic_texname);

        Material::MaterialDesc newMaterialDesc;
        newMaterialDesc.m_name = StrToWideStr(material.name);
        newMaterialDesc.m_resourceViews.push_back(diffuseTexture.isValid() ? diffuseTexture.getDefaultShaderResourceView() : Descriptor());
        newMaterialDesc.m_resourceViews.push_back(normalTexture.isValid() ? normalTexture.getDefaultShaderResourceView() : Descriptor());
        newMaterialDesc.m_resourceViews.push_back(roughnessTexture.isValid() ? roughnessTexture.getDefaultShaderResourceView() : Descriptor());
        newMaterialDesc.m_resourceViews.push_back(metallicTexture.isValid() ? metallicTexture.getDefaultShaderResourceView() : Descriptor());

        loadedMaterials.push_back(std::make_shared<Material>(newMaterialDesc));
    }

    auto newMesh = new Mesh();

    // Load shapes
    for (size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++)
    {
        std::vector<Mesh::Vertex> unindexedVertexBuffer;

        // Loop over faces(polygon)
        size_t numFaces = shapes[shapeIdx].mesh.num_face_vertices.size();
        size_t index_offset = 0;
        for (size_t faceIdx = 0; faceIdx < numFaces; faceIdx++)
        {
            size_t numFaceVertices = size_t(shapes[shapeIdx].mesh.num_face_vertices[faceIdx]);
            assert(numFaceVertices == 3);

            // Loop over vertices in the face.
            for (size_t vertexIdx = 0; vertexIdx < numFaceVertices; vertexIdx++)
            {
                Mesh::Vertex vertex;
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
        size_t vertex_count = meshopt_generateVertexRemap(&remap[0], NULL, index_count, &unindexedVertexBuffer[0], index_count, sizeof(Mesh::Vertex));
        std::vector<Mesh::Vertex> vertexBuffer(vertex_count);

        meshopt_remapIndexBuffer(indexBuffer.data(), NULL, index_count, &remap[0]);
        meshopt_remapVertexBuffer(vertexBuffer.data(), &unindexedVertexBuffer[0], index_count, sizeof(Mesh::Vertex), &remap[0]);
        //meshopt_optimizeVertexCache(indexBuffer.data(), indexBuffer.data(), index_count, vertex_count);
        //meshopt_optimizeOverdraw(indexBuffer.data(), indexBuffer.data(), index_count, &vertexBuffer[0].m_position.x, vertex_count, sizeof(Vertex), 1.05f);
        //meshopt_optimizeVertexFetch(vertexBuffer.data(), indexBuffer.data(), index_count, vertexBuffer.data(), vertex_count, sizeof(Vertex));

        // Calculate tangent space for vertices
        DirectX::XMFLOAT3* tan1 = new DirectX::XMFLOAT3[vertex_count];
        DirectX::XMFLOAT3* tan2 = new DirectX::XMFLOAT3[vertex_count];
        ZeroMemory(tan1, vertex_count * sizeof(DirectX::XMFLOAT3));
        ZeroMemory(tan2, vertex_count * sizeof(DirectX::XMFLOAT3));
        for (int i = 0; i < indexBuffer.size(); i += 3)
        {
            long i1 = indexBuffer[i];
            long i2 = indexBuffer[i+1];
            long i3 = indexBuffer[i+2];

            const DirectX::XMFLOAT3& v1 = vertexBuffer[i1].m_position;
            const DirectX::XMFLOAT3& v2 = vertexBuffer[i2].m_position;
            const DirectX::XMFLOAT3& v3 = vertexBuffer[i3].m_position;

            const DirectX::XMFLOAT2& w1 = vertexBuffer[i1].m_uvs;
            const DirectX::XMFLOAT2& w2 = vertexBuffer[i2].m_uvs;
            const DirectX::XMFLOAT2& w3 = vertexBuffer[i3].m_uvs;

            float x1 = v2.x - v1.x;
            float x2 = v3.x - v1.x;
            float y1 = v2.y - v1.y;
            float y2 = v3.y - v1.y;
            float z1 = v2.z - v1.z;
            float z2 = v3.z - v1.z;

            float s1 = w2.x - w1.x;
            float s2 = w3.x - w1.x;
            float t1 = w2.y - w1.y;
            float t2 = w3.y - w1.y;

            float r = 1.0F / (s1 * t2 - s2 * t1);
            DirectX::XMFLOAT3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                (t2 * z1 - t1 * z2) * r);
            DirectX::XMFLOAT3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                (s1 * z2 - s2 * z1) * r);

            tan1[i1].x += sdir.x;
            tan1[i1].y += sdir.y;
            tan1[i1].z += sdir.z;
            tan1[i2].x += sdir.x;
            tan1[i2].y += sdir.y;
            tan1[i2].z += sdir.z;
            tan1[i3].x += sdir.x;
            tan1[i3].y += sdir.y;
            tan1[i3].z += sdir.z;

            tan2[i1].x += tdir.x;
            tan2[i1].y += tdir.y;
            tan2[i1].z += tdir.z;
            tan2[i2].x += tdir.x;
            tan2[i2].y += tdir.y;
            tan2[i2].z += tdir.z;
            tan2[i3].x += tdir.x;
            tan2[i3].y += tdir.y;
            tan2[i3].z += tdir.z;
        }

        //Orthogonalize and store in vertices
        for (int i = 0; i < vertex_count; i++)
        {
            const DirectX::XMFLOAT3& n = vertexBuffer[i].m_normal;
            const DirectX::XMFLOAT3& t = tan1[i];
            const DirectX::XMFLOAT3& b = tan2[i];
            // Gram-Schmidt orthogonalize
            DirectX::XMStoreFloat3(&vertexBuffer[i].m_tangent, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&t), DirectX::XMVectorScale(DirectX::XMLoadFloat3(&n), DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&n), DirectX::XMLoadFloat3(&t)).m128_f32[0]))));
            DirectX::XMStoreFloat3(&vertexBuffer[i].m_bitangent, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&b), DirectX::XMVectorScale(DirectX::XMLoadFloat3(&n), DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&n), DirectX::XMLoadFloat3(&b)).m128_f32[0]))));
        }

        newMesh->m_submeshes.emplace_back();
        Mesh::Submesh& newSubmesh = newMesh->m_submeshes.back();
        newSubmesh.m_name = shapes[shapeIdx].name;
        newSubmesh.m_vertexBuffer = std::make_unique<VertexBuffer>(vertexBuffer.data(), sizeof(Mesh::Vertex), static_cast<UINT>(vertexBuffer.size()));
        newSubmesh.m_indexBuffer = std::make_unique<IndexBuffer>(indexBuffer.data(), static_cast<UINT>(index_count));
        newSubmesh.m_shaderFilepath = shaderFilepath;
        if (shapes[shapeIdx].mesh.material_ids[0] >= 0 && shapes[shapeIdx].mesh.material_ids[0] < loadedMaterials.size())
        {
            newSubmesh.m_material = loadedMaterials[shapes[shapeIdx].mesh.material_ids[0]];
        }
        else
        {
            newSubmesh.m_material = std::make_shared<Material>();
        }
    }

    return newMesh;
}
