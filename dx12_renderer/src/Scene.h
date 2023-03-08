#pragma once

#include <string>
#include <memory>

#include <Camera.h>
#include <geometry/Mesh.h>

class Scene
{
public:
    Scene( wchar_t const* name );
    ~Scene();

    Camera& getCamera() { return m_camera; }

    void record( wchar_t const* techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList );
    void addGeometry( Mesh* geometry );

private:
    std::wstring m_name;

    Camera m_camera;

    std::vector< std::shared_ptr<Mesh> > m_geometry;
};

