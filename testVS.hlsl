cbuffer CBuffer : register(b0)
{
    matrix viewProjection;
};

//cbuffer CBuffer : register(b0)
//{
//    matrix model;
//    matrix modelView;
//    matrix modelViewProjection;
//};

struct VSOut
{
    //float3 wpos : POSITION;
    float2 tc : TEXCOORD;
    float4 svpos : SV_POSITION;
};

VSOut main(float3 pos : POSITION, float2 tc : TEXCOORD)
{
    VSOut vso;
    vso.tc = tc;
    //vso.wpos = pos;
    //vso.svpos = mul(float4(pos, 1.0f), modelViewProjection);
    vso.svpos = mul(float4(pos, 0.0f), viewProjection);
    vso.svpos.z = vso.svpos.w;
    return vso;
}