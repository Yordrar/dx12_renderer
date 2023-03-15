#pragma once

#include <Manager.h>
#pragma once

#include <Manager.h>
#include <geometry/Material.h>

class MaterialManager : public Manager<MaterialManager>
{
    friend class Manager<MaterialManager>;
public:
    ~MaterialManager() = default;

    std::unique_ptr<Material> const& getMaterial( wchar_t const* materialName ) const;

    void createMaterial( Material::MaterialDesc const& materialDesc );

private:
    MaterialManager();

    std::vector< std::unique_ptr<Material> > m_materials;
};
