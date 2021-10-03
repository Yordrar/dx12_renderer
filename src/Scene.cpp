#include "Scene.h"

#include <directxmath.h>

Scene::Scene()
    : m_root(nullptr)
    , m_camera( DirectX::XMVectorSet( 0, 0, 0, 0 ), DirectX::XMVectorSet( 0, 0, 1, 0 ), 90, 16.0f/9.0f )
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

void Scene::draw( Renderer::RenderContext& context )
{
    context.m_commandList->SetGraphicsRootConstantBufferView( 0, m_camera.getCameraBuffer()->getGPUBufferLocation() );
    if ( m_root )
    {
        m_root->m_drawable->draw( context );
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
