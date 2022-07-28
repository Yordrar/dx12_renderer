cbuffer cameraBuffer : register( b0, space0 )
{
    float4x4 viewProj;
    float4 cameraPosition;
};

cbuffer bindlessIndices : register( b1, space0 )
{
    uint textureIdx;
};

Buffer bufferResources[ 128 ] : register( t0, space0 );
Texture2D texture2DResources[ 128 ] : register( t0, space1 );
TextureCube textureCubeResources[ 128 ] : register( t0, space2 );

SamplerState textureSampler : register( s0, space0 )
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
