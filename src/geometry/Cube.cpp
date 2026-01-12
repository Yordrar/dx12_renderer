#include "Cube.h"

#include <geometry/Material.h>

static wchar_t const* sc_cubeMaterialName = L"renderer/cubeMaterial";

Cube::Cube( wchar_t const* shaderFilepath )
{
    Material::MaterialDesc cubeMaterialDesc;
    cubeMaterialDesc.m_name = sc_cubeMaterialName;

    std::vector<Vertex> vertexBuffer(8);
    vertexBuffer[0].m_position = Vector3(-1.0f, 1.0f, -1.0f);
    vertexBuffer[1].m_position = Vector3(1.0f, 1.0f, -1.0f);
    vertexBuffer[2].m_position = Vector3(1.0f, -1.0f, -1.0f);
    vertexBuffer[3].m_position = Vector3(-1.0f, -1.0f, -1.0f);
    vertexBuffer[4].m_position = Vector3(-1.0f, 1.0f, 1.0f);
    vertexBuffer[5].m_position = Vector3(1.0f, 1.0f, 1.0f);
    vertexBuffer[6].m_position = Vector3(1.0f, -1.0f, 1.0f);
    vertexBuffer[7].m_position = Vector3(-1.0f, -1.0f, 1.0f);

    std::vector<UINT> indexBuffer = {
        0, 1, 2,
        2, 3, 0,
        1, 5, 6,
        6, 2, 1,
        7, 6, 5,
        5, 4, 7,
        4, 0, 3,
        3, 7, 4,
        4, 5, 1,
        1, 0, 4,
        3, 2, 6,
        6, 7, 3,
    };

    m_submeshes.emplace_back();
    Submesh& newSubmesh = m_submeshes.back();
    newSubmesh.m_name = "cube";
    newSubmesh.m_vertexBuffer = std::make_unique<VertexBuffer>(vertexBuffer.data(), sizeof(Vertex), static_cast<UINT>(vertexBuffer.size()));
    newSubmesh.m_indexBuffer = std::make_unique<IndexBuffer>(indexBuffer.data(), static_cast<UINT>(indexBuffer.size()));
    newSubmesh.m_shaderFilepath = shaderFilepath;
    newSubmesh.m_material = std::make_shared<Material>(cubeMaterialDesc);
}