
cbuffer cameraBuffer
{
    float4x4 m_viewProj;
    float4 m_cameraPosition;
};

cbuffer bindlessIndices
{

};

Texture2D Texture2DTable[] : register( t0, space0 );

float4 main_vs( float4 pos : SV_POSITION, float3 cam_pos : POSITION0, float3 world_pos : POSITION1, float3 normal : NORMAL0, float2 uvs : TEXCOORDS, float3 tangent : TANGENT, float3 bitangent : BITANGENT ) : SV_POSITION
{
    float4 col = float4( 0.5f, 0.5f, 0.5f, 1.0f ) * max( dot( normalize( cam_pos - world_pos ), normalize( normal ) ), 0.0f );
    return sqrt( col );
}