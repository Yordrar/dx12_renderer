#include "IGeometry.h"

#include <pix3.h>

#include <resource/IResource.h>

IGeometry::IGeometry( std::string name, std::initializer_list<std::string> techniqueNames )
    : m_name( name )
    , m_techniqueNames( techniqueNames )
{
}

IGeometry::~IGeometry()
{
}

void IGeometry::record( std::string techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState )
{
    for ( IResource* resource : m_resources )
    {
        resource->copyDataToGPU( commandList );
    }
}

void IGeometry::addResource( IResource* resource )
{
    m_resources.push_back( resource );
}