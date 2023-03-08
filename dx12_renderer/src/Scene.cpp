#include "Scene.h"

#include <directxmath.h>

#include <Renderer.h>
#include <resource/ResourceManager.h>
#include <Camera.h>

Scene::Scene( wchar_t const* name )
    : m_name( name )
    , m_camera( Camera( std::wstring(m_name + L"_camera").c_str(), DirectX::XMVectorSet(0.0f, 0.0f, 2.0f, 0.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), 90.0f, 16.0f / 9.0f ) )
{
    
}

Scene::~Scene()
{

}

void Scene::record( wchar_t const* techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList )
{
    m_camera.setCameraBufferView( commandList );

    for ( std::shared_ptr<Mesh>& geometry : m_geometry )
    {
        std::vector<std::wstring> const& techniqueNames = geometry->getTechniqueNames();
        if ( std::find( techniqueNames.cbegin(), techniqueNames.cend(), techniqueName ) != geometry->getTechniqueNames().cend() )
        {
            geometry->record( techniqueName, commandList );
        }
    }
}

void Scene::addGeometry( Mesh* mesh )
{
    std::shared_ptr<Mesh> newMesh( mesh );
    m_geometry.push_back( newMesh );
}
