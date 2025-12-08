#pragma once

#include "types.h"

#define PI 3.14159265359;

struct DrawConstants
{
    uint passBufferIndex;
    uint cameraBufferIndex;
    uint materialBufferIndex;
    uint geometryBufferIndex;
    uint pointLightBufferIndex;
    uint directionalLightBufferIndex;
    uint spotLightBufferIndex;
    uint numPointLights;
    uint numDirectionalLights;
    uint numSpotLights;
};
ConstantBuffer<DrawConstants> drawConstants : register(b0, space0);

template<typename buffer_type>
buffer_type getBuffer(uint index)
{
    ByteAddressBuffer buffer = ResourceDescriptorHeap[index];
    return buffer.Load<buffer_type>(0);
}

template<typename buffer_type>
buffer_type getPassBuffer()
{
    return getBuffer<buffer_type>(drawConstants.passBufferIndex);
}

CameraData getCameraData()
{
    return getBuffer<CameraData>(drawConstants.cameraBufferIndex);
}

template<typename buffer_type>
buffer_type getMaterialBuffer()
{
    return getBuffer<buffer_type>(drawConstants.materialBufferIndex);
}

StructuredBuffer<PointLight> getPointLightBuffer()
{
    return ResourceDescriptorHeap[drawConstants.pointLightBufferIndex];
}

StructuredBuffer<DirectionalLight> getDirectionalLightBuffer()
{
    return ResourceDescriptorHeap[drawConstants.directionalLightBufferIndex];
}

StructuredBuffer<SpotLight> getSpotLightBuffer()
{
    return ResourceDescriptorHeap[drawConstants.spotLightBufferIndex];
}