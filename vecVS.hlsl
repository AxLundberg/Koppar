
cbuffer CBuffer
{
    matrix model;
    matrix modelview;
    matrix modelViewProjection;
};

struct VSOut
{
    float2 tc : TEXCOORD;
    float4 svpos : SV_POSITION;
};

VSOut main(float3 pos : POSITION, float2 tc : TEXCOORD)
{
    VSOut vso;
    vso.tc = tc;
    vso.svpos = mul(float4(pos, 1.0f), modelViewProjection);
    
    return vso;
}