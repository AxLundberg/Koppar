cbuffer CBuffer : register(b0)
{
	matrix model;
    matrix modelview;
    matrix modelViewProjection;
};

cbuffer camCBuf : register(b1)
{
    matrix lightViewProj;
};

struct VSOutput
{
	float3 view_pos : POSITION;
    float4 shadow_pos : SHADOWPOSITION;
    float3 normal : NORMAL;
    float2 tc : TEXCOORD;
    float3x3 modelview : MATRIX;
    float4 sv_pos : SV_POSITION;
};

VSOutput main(float3 pos : POSITION, float2 tc : TEXCOORD, float3 normal : NORMAL) 
{
    const float4 vertexWpos = (float4) mul(float4(pos, 1.0f), model);  //Vertex World pos
    const float4 vertexLpos = (float4) mul(vertexWpos, lightViewProj); //Vertex pos in homogenous light space
    
	VSOutput output;
    output.shadow_pos = (vertexLpos * float4(0.5f, -0.5f, 1.0f, 1.0f)) +
                        (vertexLpos.w * float4(0.5f, 0.5f, 0.0f, 0.0f));
    
	output.view_pos = (float3)mul( float4( pos, 1.0f ), modelview );
    output.normal = normalize(mul(normal, (float3x3) modelview));
	output.sv_pos = mul( float4(pos, 1.0f), modelViewProjection );
    output.modelview = (float3x3) modelview;
    output.tc = tc;
    return output;
}