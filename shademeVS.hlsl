cbuffer CBuf : register(b0)
{
    matrix model;
    matrix modelview;
    matrix modelViewProj;
};

cbuffer camCBuf : register(b1)
{
    matrix lightViewProj;
};

struct VSoutput
{
    float3 view_pos : POSITION;
    float4 shadow_pos : SHADOWPOSITION;
    float4 sv_pos : SV_POSITION;
};

VSoutput main( float3 pos : POSITION )
{
    VSoutput output;
    
    const float4 vertexWpos = (float4) mul(float4(pos, 1.0f), model);  //Vertex World pos
    const float4 vertexLpos = (float4) mul(vertexWpos, lightViewProj); //Vertex pos in homogenous light space
    
    output.sv_pos = (float4) mul(float4(pos, 1.0f), modelViewProj);
    output.view_pos = (float3) mul(float4(pos, 1.0f), modelview);
    output.shadow_pos = (vertexLpos   * float4(0.5f, -0.5f, 1.0f, 1.0f)) +
                        (vertexLpos.w * float4(0.5f, 0.5f, 0.0f, 0.0f));
    
	return output;
}