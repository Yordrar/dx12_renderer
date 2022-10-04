#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>
#include <string>

#include <Scene.h>

class RenderPass
{
public:
    RenderPass( std::string name, std::initializer_list<Scene> scenes );
    ~RenderPass();

private:
    std::vector<Scene> m_scenes;
};

