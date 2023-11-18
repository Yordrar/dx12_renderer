#include "common.hlsli"

#if defined(TEST)

struct PassIndices
{
    uint testTexture;
};

[numthreads(8, 8, 1)]
void test_cs( uint3 threadID : SV_DispatchThreadID )
{
    PassIndices indices = getPassBuffer<PassIndices>();
    RWTexture2D<float4> texture = ResourceDescriptorHeap[indices.testTexture];
    texture[threadID.xy] = float4(threadID.xy / float2(255,255), 0.0, 1.0);
}

#endif

#if defined(MIPMAP_GENERATOR)

struct PassIndices
{
    uint textureMip0Index;
    uint textureMip1Index;
};

[numthreads(8, 8, 1)]
void mipmap_generator_cs( uint3 threadID : SV_DispatchThreadID )
{
    PassIndices indices = getPassBuffer<PassIndices>();
    Texture2D textureMip0 = ResourceDescriptorHeap[indices.textureMip0Index];
    RWTexture2D<float4> textureMip1 = ResourceDescriptorHeap[indices.textureMip1Index];
    float2 uv = (1.0f/877.0f) * ( threadID.xy + 0.5 );
    SamplerState samp = SamplerDescriptorHeap[0];
    float4 color = textureMip0.SampleLevel(samp, uv, 0);
    textureMip1[threadID.xy] = color;
}

#endif