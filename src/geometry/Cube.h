#pragma once

#include <geometry/Mesh.h>

class Cube : public Mesh
{
public:
    Cube( wchar_t const* shaderFilepath );
    ~Cube() = default;

    void addResourceView(Descriptor descriptor) { m_submeshes[0].m_material->addResourceView(descriptor); }
};