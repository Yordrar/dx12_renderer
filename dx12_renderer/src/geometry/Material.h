#pragma once

#include <vector>
#include <string>

class Material
{
public:
    Material( std::initializer_list<std::wstring> resourceNames );
    ~Material();

private:
    std::vector<std::wstring> m_resourceNames;
};