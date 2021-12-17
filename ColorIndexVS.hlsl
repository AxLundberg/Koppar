cbuffer CBuf
{
    matrix model;
    matrix modelview;
    matrix modelViewProj;
};
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
};

VS_OUTPUT main(float3 in_pos : POSITION)
{
    VS_OUTPUT output;
    output.pos = mul(float4(in_pos, 1.0f), modelViewProj);
    return output;
}

