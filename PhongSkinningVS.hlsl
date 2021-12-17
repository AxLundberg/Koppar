cbuffer CBuffer : register(b0)
{
    matrix model;
    matrix modelView;
    matrix modelViewProjection;
    matrix modelbone[256];
};

cbuffer camCBuf : register(b1)
{
    matrix lightViewProj;
};


struct VSOutput
{
    float3 world_pos : POSITION;
    float4 shadow_pos : SHADOWPOSITION;
    float3 normal : NORMAL;
    float2 tc : TEXCOORD;
    float4 svpos : SV_POSITION;
};

VSOutput main(float3 pos : POSITION, float3 normal : NORMAL, float2 tc : TEXCOORD, int4 bones : BLENDINDICES0, float4 weights : BLENDWEIGHT)
{
    VSOutput output;

    matrix mat;
    for (uint i = 0; i < 4; i++)
    {
        if (bones[i] != -1)
        {
            mat += modelbone[bones[i]] * weights[i];
        }     
    }
    pos = (float3) mul(float4(pos, 1.0f), mat);
    
    const float4 vertexWpos = (float4) mul(float4(pos, 1.0f), model);  //Vertex World pos
    const float4 vertexLpos = (float4) mul(vertexWpos, lightViewProj); //Vertex pos in homogenous light space
    
    output.shadow_pos = (vertexLpos * float4(0.5f, -0.5f, 1.0f, 1.0f)) +
                        (vertexLpos.w * float4(0.5f, 0.5f, 0.0f, 0.0f));
    output.world_pos = (float3) mul(float4(pos, 1.0f), modelView);
    output.svpos = mul(float4(pos, 1.0f), modelViewProjection);
    
    normal = normalize(mul(normal, (float3x3) mat));
    output.normal = mul(normal, (float3x3) modelView);
    output.tc = tc;
    return output;
}