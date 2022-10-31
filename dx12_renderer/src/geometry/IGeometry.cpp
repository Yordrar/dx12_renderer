#include "IGeometry.h"

IGeometry::IGeometry( std::string name, std::initializer_list<std::string> renderPassNames )
    : m_name( name )
    , m_renderPassNames( renderPassNames )
{
}
