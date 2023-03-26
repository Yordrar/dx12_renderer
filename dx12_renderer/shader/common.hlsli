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
Texture1D texture1DResources[DESCRIPTORS_IN_HEAPS] : register(t0, space1);
Texture2D texture2DResources[DESCRIPTORS_IN_HEAPS] : register(t0, space2);
Texture3D texture3DResources[DESCRIPTORS_IN_HEAPS] : register(t0, space3);
TextureCube textureCubeResources[DESCRIPTORS_IN_HEAPS] : register(t0, space4);

RWByteAddressBuffer rwBufferResources[DESCRIPTORS_IN_HEAPS] : register(u0, space0);
RWTexture1D<float4> rwTexture1DResources[DESCRIPTORS_IN_HEAPS] : register(u0, space1);
RWTexture2D<float4> rwTexture2DResources[DESCRIPTORS_IN_HEAPS] : register(u0, space2);
RWTexture3D<float4> rwTexture3DResources[DESCRIPTORS_IN_HEAPS] : register(u0, space3);

sampler samplerResources[DESCRIPTORS_IN_HEAPS] : register(s0, space0);

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
