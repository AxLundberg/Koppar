cbuffer CBuf : register(b0)
{
    matrix model;
    matrix modelview;
    matrix modelViewProjection;
};

cbuffer camCBuf : register(b1)
{
    matrix lightViewProj;
};

struct VS_INPUT
{
    float3 pos      : POSITION;
    float3 tangent  : TANGENT;
    float3 binormal : BINORMAL;
    float3 normal   : NORMAL;
    float2 tc       : TEXCOORD;
};

struct VS_OUTPUT
{
    float3 view_pos   : POSITION;
    float4 shadow_pos : SHADOWPOSITION;
    float3 viewnormal : NORMAL;
    float2 tc         : TEXCOORD;
    float3x3 TBN      : MATRIX;
    float4 sv_pos     : SV_POSITION;
};
    
VS_OUTPUT main(VS_INPUT input)
{    
    float3 t = normalize(mul(input.tangent, (float3x3) modelview));
    float3 n = normalize(mul(input.normal, (float3x3) modelview));
    //float3 b = normalize(mul(input.binormal, (float3x3) model));
    //t = normalize(t - dot(t, n) * n);
    float3 b = normalize(cross(t, n));
    float3x3 tbn = float3x3(t, b, n);
    
    const float4 vertexWpos = (float4) mul(float4(input.pos, 1.0f), model); //Vertex World pos
    const float4 vertexLpos = (float4) mul(vertexWpos, lightViewProj);      //Vertex pos in homogenous light space
    
    VS_OUTPUT output;
    
    output.shadow_pos = (vertexLpos * float4(0.5f, -0.5f, 1.0f, 1.0f)) +
                        (vertexLpos.w * float4(0.5f, 0.5f, 0.0f, 0.0f));
    
    output.view_pos   = (float3) mul(float4(input.pos, 1.0f), modelview);
    
    output.sv_pos     = mul(float4(input.pos, 1.0f), modelViewProjection);
    
    output.viewnormal = mul(input.normal, (float3x3) modelview);
    
    output.tc  = input.tc;
    output.TBN = tbn;
    
    return output;
}