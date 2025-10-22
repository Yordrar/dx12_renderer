#pragma once

#include <geometry/Mesh.h>

class MeshImporter
{
public:
    static std::shared_ptr<Mesh> createEmpty();
    static std::shared_ptr<Mesh> createFromObjFile(wchar_t const* objFilepath, wchar_t const* shaderFilepath);
};