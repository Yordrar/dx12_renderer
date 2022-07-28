#include "Scene.h"

#include <directxmath.h>

Scene::Scene()
    : m_root(nullptr)
    , m_camera( DirectX::XMVectorSet( 0, 0, 3, 0 ), DirectX::XMVectorSet( 0, 0, -1, 0 ), 70, 16.0f/9.0f )
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
        return newSceneNode;
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
    context.m_commandList->SetGraphicsRootConstantBufferView( 0, m_camera.getCameraBuffer()->getGPUVirtualAddress() );
    m_camera.updateCameraBuffers( context );
    if ( m_root )
    {
        m_root->m_drawable->draw( context );
        for ( SceneNode* node : m_root->m_children )
        {
            node->draw( context );
        }
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

void Scene::SceneNode::draw( Renderer::RenderContext& context )
{
    if ( m_drawable )
    {
        m_drawable->draw( context );
        for ( SceneNode* node : m_children )
        {
            node->draw( context );
        }
    }
}
