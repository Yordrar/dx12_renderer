#include "FullscreenTriangle.h"

#include <geometry/Material.h>

static wchar_t const* sc_fstMaterialName = L"renderer/fstMaterial";

FullscreenTriangle::FullscreenTriangle( wchar_t const* shaderFilepath )
{
    Material::MaterialDesc fstMaterialDesc;
    fstMaterialDesc.m_name = sc_fstMaterialName;

    std::vector<Vertex> vertexBuffer(3);
    Vertex v1;
    v1.m_position = Vector3(-1, 1, 0);
    v1.m_uvs.x = 0;
    v1.m_uvs.y = 0;
    Vertex v2;
    v2.m_position = Vector3(3, 1, 0);
    v2.m_uvs.x = 2;
    v2.m_uvs.y = 0;
    Vertex v3;
    v3.m_position = Vector3(-1, -3, 0);
    v3.m_uvs.x = 0;
    v3.m_uvs.y = 2;
    vertexBuffer.push_back(v1);
    vertexBuffer.push_back(v2);
    vertexBuffer.push_back(v3);

    m_submeshes.emplace_back();
    Submesh& newSubmesh = m_submeshes.back();
    newSubmesh.m_name = "fullscreenTriangle";
    newSubmesh.m_vertexBuffer = std::make_unique<VertexBuffer>(vertexBuffer.data(), sizeof(Vertex), static_cast<UINT>(vertexBuffer.size()));
    newSubmesh.m_indexBuffer = nullptr;
    newSubmesh.m_shaderFilepath = shaderFilepath;
    newSubmesh.m_material = std::make_shared<Material>(fstMaterialDesc);
}