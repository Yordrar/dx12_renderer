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

        void draw( Renderer::RenderContext& context );

        IDrawable* m_drawable = nullptr;
        std::vector<SceneNode*> m_children;
    };

public:
    Scene();
    ~Scene();

    SceneNode* addDrawable( IDrawable* drawable );
    Camera& getCamera() { return m_camera; }

    void draw( Renderer::RenderContext& context );

private:
    SceneNode* m_root;
    Camera m_camera;
};

