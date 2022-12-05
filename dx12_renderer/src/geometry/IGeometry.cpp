#include "IGeometry.h"

#include <pix3.h>

#include <resource/IResource.h>
#include <resource/ResourceManager.h>

IGeometry::IGeometry( std::string name, std::initializer_list<std::string> techniqueNames )
    : m_name( name )
    , m_techniqueNames( techniqueNames )
    , m_isDirty( true )
{
    m_bindlessIndices = ResourceManager::it().createConstantBuffer( name + "_bindlessBuffer", nullptr, 0 );
}

IGeometry::~IGeometry()
{
}

void IGeometry::record( std::string techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState )
{
    if ( m_isDirty )
    {
        m_isDirty = false;

        for ( IResource* resource : m_resources )
        {
            resource->copyDataToGPU( commandList );
        }

        ResourceManager::it().destroyResource( m_name + "_bindlessBuffer" );
        m_bindlessIndices = ResourceManager::it().createConstantBuffer( m_name + "_bindlessBuffer", m_resourceIndices.data(), m_resourceIndices.size() * sizeof(float) );
        m_bindlessIndices->copyDataToGPU( commandList );
    }
    commandList->SetGraphicsRootConstantBufferView( 1, m_bindlessIndices->getGPUVirtualAddress() );
}

void IGeometry::addResource( IResource* resource )
{
    m_isDirty = true;
    m_resources.push_back( resource );
    m_resourceIndices.push_back( resource->getDescriptorHeapIndex() );
}