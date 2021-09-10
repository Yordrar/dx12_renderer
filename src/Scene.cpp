#include "Scene.h"

#include <directxmath.h>

Scene::Scene()
    : m_root(nullptr)
    , m_camera()
{
}

Scene::~Scene()
{
    delete m_root;
}

Scene::SceneNode* Scene::addDrawable( IDrawable* drawable )
{
    if ( m_root )
    {
        Scene::SceneNode* newSceneNode = new SceneNode();
        newSceneNode->m_drawable = drawable;
        m_root->m_children.push_back( newSceneNode );
    }
    else
    {
        m_root = new SceneNode();
        m_root->m_drawable = drawable;
        return m_root;
    }
}

Scene::SceneNode::~SceneNode()
{
    for ( SceneNode* node : m_children )
    {
        delete node;
    }
    delete m_drawable;
}
