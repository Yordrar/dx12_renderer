#include "Scene.h"

#include <directxmath.h>

#include <Renderer.h>
#include <resource/ResourceManager.h>
#include <resource/Texture.h>
#include <Camera.h>
#include <geometry/IGeometry.h>

Scene::Scene( std::wstring name )
    : m_camera( Camera( name + L"_camera", DirectX::XMVectorSet(0.0f, 0.0f, 2.0f, 0.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 90.0f, 16.0f / 9.0f ) )
{
    
}

Scene::~Scene()
{

}

void Scene::record( std::wstring techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState )
{
    m_camera.setCameraBufferView( commandList );

    for ( std::shared_ptr<IGeometry>& geometry : m_geometry )
    {
        if ( geometry->getTechniqueNames().find( techniqueName ) != geometry->getTechniqueNames().end() )
        {
            geometry->record( techniqueName, commandList, pipelineState );
        }
    }
}

void Scene::addGeometry( IGeometry* geometry )
{
    std::shared_ptr<IGeometry> newGeometry( geometry );
    m_geometry.push_back( newGeometry );
}
