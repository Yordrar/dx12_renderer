#include "Scene.h"

#include <resource/ResourceManager.h>
#include <resource/Texture.h>

Scene::Scene( std::string name,
              std::string renderTarget,
              std::string depthStencilTarget )
    : m_renderTarget( ResourceManager::it().getResource<Texture>( renderTarget ) )
{

}

Scene::~Scene()
{

}
