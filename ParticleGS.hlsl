cbuffer CBuf
{
    float3 camPos;
    matrix model;
    matrix modelViewProjection;
}

struct GS_INPUT
{
    float3 inpos : POSITION;
    float size : PSIZE;
    float4 svpos : SV_Position;
};

struct GS_OUTPUT
{
    float2 texcoord : TC;
    float4 svpos : SV_Position;
};

[maxvertexcount(4)]
void main(
	point GS_INPUT input[1], 
	inout TriangleStream< GS_OUTPUT > output)
{
    float3 primitiveNormal = (float3) mul(float4(input[0].inpos, 1.0f), model) - camPos;
    primitiveNormal = normalize(primitiveNormal);
    float3 leftVector = normalize(cross(float3(0.0f, 1.0f, 0.0f), primitiveNormal)) *  (input[0].size / 2 );
    float3 upVector = normalize(cross(primitiveNormal, leftVector)) * (input[0].size / 2);
    //float3 rightVector = normalize(cross(primitiveNormal, upVector)) * (input[0].size / 2);
    
    float2 texCoord[4];
    texCoord[0] = float2(0, 1);
    texCoord[1] = float2(1, 1);
    texCoord[2] = float2(0, 0);
    texCoord[3] = float2(1, 0);
    
    
    float4 vertices[4];
    vertices[0] = float4(input[0].inpos + leftVector - upVector, 1.0f);
    vertices[1] = float4(input[0].inpos - leftVector - upVector, 1.0f);
    vertices[2] = float4(input[0].inpos + leftVector + upVector, 1.0f);
    vertices[3] = float4(input[0].inpos - leftVector + upVector, 1.0f);

    GS_OUTPUT GS_output;
	for (uint j = 0; j < 4; j++)
	{
		GS_output.texcoord = texCoord[j];
        GS_output.svpos = mul(vertices[j], modelViewProjection);
		output.Append(GS_output);
	}
}