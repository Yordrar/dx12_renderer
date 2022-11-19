#pragma once

#include <string>
#include <memory>

#include <Camera.h>
#include <geometry/IGeometry.h>

class Scene
{
public:
    Scene( std::string name );
    ~Scene();

    Camera& getCamera() { return m_camera; }

    void record( std::string techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState );
    void addGeometry( IGeometry* geometry );

private:
    Camera m_camera;

    std::vector< std::shared_ptr<IGeometry> > m_geometry;
};

