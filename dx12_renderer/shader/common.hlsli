#define DESCRIPTORS_IN_HEAPS 100000

cbuffer passBuffer : register(b0, space0)
{
    uint passBufferIndex;
};

cbuffer cameraBuffer : register(b1, space0)
{
    float4x4 viewProjMatrix;
    float4x4 inverseViewProjMatrix;
    float4 cameraPosition;
};

cbuffer materialBuffer : register(b2, space0)
{
    uint bindlessIndicesBufferIndex;
};

cbuffer geometryBuffer : register(b3, space0)
{
    float4x4 modelMatrix;
    float4x4 inverseModelMatrix;
};

ByteAddressBuffer bufferResources[DESCRIPTORS_IN_HEAPS] : register(t0, space0);
Texture2D texture2DResources[DESCRIPTORS_IN_HEAPS] : register(t0, space1);
TextureCube textureCubeResources[DESCRIPTORS_IN_HEAPS] : register(t0, space2);
sampler sampler2DResources[DESCRIPTORS_IN_HEAPS] : register(s0, space0);

template<typename buffer_type>
buffer_type getBuffer(uint index)
{
    ByteAddressBuffer buffer = bufferResources[index];
    return buffer.Load<buffer_type>(0);
}

template<typename buffer_type>
buffer_type getBindlessIndicesBuffer()
{
    return getBuffer<buffer_type>(bindlessIndicesBufferIndex);
}

template<typename buffer_type>
buffer_type getPassIndicesBuffer()
{
    return getBuffer<buffer_type>(passBufferIndex);
}
