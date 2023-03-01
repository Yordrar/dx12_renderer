#include "IGeometry.h"

#include <algorithm>

#include <resource/Descriptor.h>
#include <resource/ResourceManager.h>

IGeometry::IGeometry( wchar_t const* name, std::initializer_list<wchar_t const*> techniqueNames )
    : m_name( name )
{
    for ( wchar_t const* techniqueName : techniqueNames )
    {
        m_techniqueNames.insert( techniqueName );
    }
}

IGeometry::~IGeometry()
{
}

void IGeometry::record( wchar_t const* techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList )
{
}