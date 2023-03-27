#include "common.hlsli"

#if defined(TEST)

struct PassIndices
{
    uint testTexture;
};

[numthreads(8, 8, 1)]
void test_cs( uint3 threadID : SV_DispatchThreadID )
{
    PassIndices indices = getPassIndicesBuffer<PassIndices>();
    RWTexture2D<float4> texture = rwTexture2DResources[indices.testTexture];
    texture[threadID.xy] = float4(threadID.xy / float2(255,255), 0.0, 1.0);
}

#endif