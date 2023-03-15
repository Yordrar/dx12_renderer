#pragma once

#include <Camera.h>
#include <geometry/Mesh.h>

class Scene
{
public:
    Scene( wchar_t const* name );
    ~Scene();

    Camera& getCamera() { return m_camera; }
    std::vector< std::shared_ptr<Mesh> > const& getMeshes() const { return m_meshes; }

    void addMesh( Mesh* geometry );

private:
    std::wstring m_name;

    Camera m_camera;

    std::vector< std::shared_ptr<Mesh> > m_meshes;
};

