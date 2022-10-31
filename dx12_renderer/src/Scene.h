#pragma once

#include <string>
#include <memory>

#include <Camera.h>

class IGeometry;
class Texture;

class Scene
{
public:
    Scene( std::string name );
    ~Scene();

    Camera& getCamera() { return m_camera; }

    void addGeometry( IGeometry* geometry );
    void getGeometryForTechnique( std::string renderPassName, std::vector< std::shared_ptr<IGeometry> >& outGeometry );

private:
    Camera m_camera;

    std::vector< std::shared_ptr<IGeometry> > m_geometry;
};

