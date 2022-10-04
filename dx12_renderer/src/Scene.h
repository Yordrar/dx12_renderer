#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>

#include <Camera.h>

class Scene
{
public:
    Scene( std::string name,
           std::string renderTarget,
           std::string depthStencilTarget );
    ~Scene();

private:
    Camera m_camera;

    std::shared_ptr<IResource> m_renderTarget;
    std::shared_ptr<IResource> m_depthStencilTarget;
};

