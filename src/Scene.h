#pragma once

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <drawable/IDrawable.h>
#include <Camera.h>

class Scene
{
    struct SceneNode
    {
        ~SceneNode();
        IDrawable* m_drawable = nullptr;
        std::vector<SceneNode*> m_children;
    };

public:
    Scene();
    ~Scene();

    SceneNode* addDrawable( IDrawable* drawable );

private:
    SceneNode* m_root;
    Camera m_camera;
};

