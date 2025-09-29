#pragma once

#include <geometry/Mesh.h>

class FullscreenTriangle : public Mesh
{
public:
    FullscreenTriangle( wchar_t const* shaderFilepath );
    ~FullscreenTriangle() = default;

    void addResourceView(Descriptor descriptor) { m_submeshes[0].m_material->addResourceView(descriptor); }
};