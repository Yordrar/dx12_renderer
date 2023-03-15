#include "MaterialManager.h"

MaterialManager::MaterialManager()
{

}

std::unique_ptr<Material> const& MaterialManager::getMaterial( wchar_t const* materialName ) const
{
    for ( std::unique_ptr<Material> const& material : m_materials )
    {
        if ( material->getName() == materialName )
        {
            return material;
        }
    }
    return nullptr;
}

void MaterialManager::createMaterial( Material::MaterialDesc const& materialDesc )
{
    m_materials.push_back( std::make_unique<Material>( materialDesc ) );
}
