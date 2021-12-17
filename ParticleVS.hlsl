struct Particle
{
    float3 position;
    float3 direction;
    float2 OGpos;
    float size;
    float velocity;
    uint time;
};
StructuredBuffer<Particle> ParticleState;

struct VS_input
{
    uint vertexID : SV_vertexID;
};
	
struct VS_ouput
{
    float3 position : POSITION;
    float4 svposition : SV_Position;
    float size : PSIZE;
};

VS_ouput main( VS_input input )
{
    VS_ouput output;
    output.position = ParticleState[input.vertexID].position;
    output.svposition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.size = ParticleState[input.vertexID].size;
	return output;
}