cbuffer cameraBuffer : register(b0, space0)
{
    float4x4 viewProj;
    float4 cameraPosition;
};

struct Light
{
};

StructuredBuffer<Light> lightBuffer : register(b1, space0);

Buffer bufferResources[1024] : register(t0, space0);
Texture2D texture2DResources[1024] : register(t0, space1);
TextureCube textureCubeResources[1024] : register(t0, space2);

SamplerState textureSampler : register(s0, space0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
