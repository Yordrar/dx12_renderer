cbuffer cameraBuffer
{
    float4x4 m_viewProj;
    float4 m_cameraPosition;
};

cbuffer bindlessIndices
{

};

Texture2D Texture2DTable[] : register( t0, space0 );

float4 main_ps( float4 pos : SV_POSITION ) : SV_Target
{
    return float4( 1.0, 0.0, 0.0, 1.0 );
}