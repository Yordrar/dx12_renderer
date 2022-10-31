#include "Scene.h"

#include <directxmath.h>

#include <Renderer.h>
#include <resource/ResourceManager.h>
#include <resource/Texture.h>
#include <Camera.h>
#include <geometry/IGeometry.h>

Scene::Scene( std::string name )
    : m_camera( Camera( name+"_Camera", DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 90.0f, 16.0f / 9.0f))
{

}

Scene::~Scene()
{

}

void Scene::addGeometry( IGeometry* geometry )
{
    std::shared_ptr<IGeometry> newGeometry( geometry );
    m_geometry.push_back( newGeometry );
}

void Scene::getGeometryForTechnique( std::string renderPassName, std::vector< std::shared_ptr<IGeometry> >& outGeometry )
{
    for ( std::shared_ptr<IGeometry>& geometry : m_geometry )
    {
        if ( geometry->getRenderPassNames().find( renderPassName ) != geometry->getRenderPassNames().end() )
        {
            outGeometry.push_back( geometry );
        }
    }
}
