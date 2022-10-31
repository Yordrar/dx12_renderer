#pragma once

#include <vector>
#include <string>

class Material
{
public:
    Material( std::initializer_list<std::string> resourceNames );
    ~Material();

private:
    std::vector<std::string> m_resourceNames;
};