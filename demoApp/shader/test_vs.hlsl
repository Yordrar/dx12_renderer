cbuffer cameraBuffer
{
    float4x4 m_viewProj;
    float4 m_cameraPosition;
};

cbuffer bindlessIndices
{

};

Texture2D Texture2DTable[] : register( t0, space0 );

float4 main_vs( float3 pos : POSITION ) : SV_POSITION
{
    return mul( float4( pos, 1.0f ), m_viewProj );
}