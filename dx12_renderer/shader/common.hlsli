cbuffer cameraBuffer : register(b0, space0)
{
    float4x4 viewProj;
    float4 cameraPosition;
};

Texture2D texture2DResources[1024] : register(t0, space0);
TextureCube textureCubeResources[1024] : register(t0, space1);
sampler sampler2DResources[1024] : register(s0, space0);
