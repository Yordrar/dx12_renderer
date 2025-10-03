#pragma once

#include <geometry/Mesh.h>

class MeshImporter
{
public:
    static Mesh* createEmpty();
    static Mesh* createFromObjFile(wchar_t const* objFilepath, wchar_t const* shaderFilepath);
};