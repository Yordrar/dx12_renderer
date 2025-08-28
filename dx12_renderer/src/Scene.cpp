#include "Scene.h"

#include <Renderer.h>
#include <resource/ResourceManager.h>
#include <Camera.h>

Scene::Scene( wchar_t const* name )
    : m_name( name )
    , m_camera( Camera( std::wstring(m_name + L"_camera").c_str(), DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f) ) )
{
    
}

Scene::~Scene()
{

}

void Scene::addMesh( Mesh* mesh )
{
    std::shared_ptr<Mesh> newMesh( mesh );
    m_meshes.push_back( newMesh );
}

std::vector<std::shared_ptr<Mesh>> const& Scene::getMeshes()
{
    return m_meshes;
}
