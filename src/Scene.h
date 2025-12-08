#pragma once

#include <d3d12.h>

#include <Camera.h>
#include <LightDefs.h>
#include <geometry/Mesh.h>

class Scene
{
public:
    Scene( wchar_t const* name );
    ~Scene();

    std::wstring const& getName() const { return m_name; }

    Camera& getCamera() { return m_camera; }
    ResourceHandle getCameraBufferResource() const { return m_camera.getGPUBufferResource(); }

    void addMesh( Mesh* mesh );
    std::vector<Mesh*> const& getMeshes();

    void addPointLight(PointLight const& light);
    Descriptor getPointLightBufferSRV() const { return m_pointLightBufferSRV; }
    uint32_t getNumPointLights() const { return (uint32_t)m_pointLights.size(); }

    void addDirectionalLight(DirectionalLight const& light);
    Descriptor getDirectionalLightBufferSRV() const { return m_directionalLightBufferSRV; }
    uint32_t getNumDirectionalLights() const { return (uint32_t)m_directionalLights.size(); }

    void addSpotLight(SpotLight const& light);
    Descriptor getSpotLightBufferSRV() const { return m_spotLightBufferSRV; }
    uint32_t getNumSpotLights() const { return (uint32_t)m_spotLights.size(); }

private:
    void recreatePointLightBuffer();
    void recreateDirectionalLightBuffer();
    void recreateSpotLightBuffer();

    std::wstring m_name;

    Camera m_camera;

    std::vector<Mesh*> m_meshes;

    std::vector<PointLight> m_pointLights;
    ResourceHandle m_pointLightBuffer;
    Descriptor m_pointLightBufferSRV;
    
    std::vector<DirectionalLight> m_directionalLights;
    ResourceHandle m_directionalLightBuffer;
    Descriptor m_directionalLightBufferSRV;

    std::vector<SpotLight> m_spotLights;
    ResourceHandle m_spotLightBuffer;
    Descriptor m_spotLightBufferSRV;
};

