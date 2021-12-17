cbuffer cbuf
{
    float alphaVariance;
    float betaVariance;
    float velocityVariance;
    float nParticles;
    float3 emitterPos;
    float emitterRadius;
    float2 initAlpha;
    float2 initBeta;
    float2 initVelocity;
    float size;
};
struct Particle
{
    float3 position;
    float3 direction;
    float2 OGpos;
    float size;
    float velocity;
    uint time;
};

Texture2D<uint> textre2D : register(t0);
Texture1D<float> textre1D : register(t1);

ConsumeStructuredBuffer<Particle> preParticles : register(u0);
AppendStructuredBuffer<Particle> postParticles : register(u1);

RWStructuredBuffer<Particle> outputData : register(u2);

#define N_PARTICLES 256
#define PARTICLE_LIFETIME 128.

[numthreads(N_PARTICLES, 1, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    Particle p;
    p = preParticles.Consume();
    
    p.position = p.position + (p.velocity * float3(cos(p.direction.x) * cos(p.direction.y), sin(p.direction.y), cos(p.direction.y) * sin(p.direction.x)));
    
    uint rand2D     = textre2D.Load(int3(uint((abs(p.position.x) + 0.01f) * 100.0f) % 256, uint((abs(p.position.z + 0.01) * 100.0f)) % 128, 0));
    uint randValue  = textre2D.Load(int3((rand2D & 0x000000FF), 131, 0));
    uint alphaV     = textre2D.Load(int3((rand2D & 0x0000FF00) >> 8, 128, 0));
    uint betaV      = textre2D.Load(int3((rand2D & 0x00FF0000) >> 16, 129, 0));
    uint veloV      = textre2D.Load(int3((rand2D & 0xFF000000) >> 24, 130, 0));
   
    p.time--;
    if (p.time > 1)
    {
        p.direction.x += asfloat(alphaV) * alphaVariance;
        p.direction.y += asfloat(betaV) * betaVariance;
        p.velocity    += asfloat(veloV) * velocityVariance;
        p.direction.z  = asfloat(randValue);
        p.size = size * (1.0f - abs(2 * p.time - PARTICLE_LIFETIME) / PARTICLE_LIFETIME);
    }
    else
    {
        p.position = float3(p.OGpos.x, emitterPos.y, p.OGpos.y) +
                           (p.velocity *
                           float3(cos(p.direction.x) * cos(p.direction.y), sin(p.direction.y), cos(p.direction.y) * sin(p.direction.x)));
        p.time = PARTICLE_LIFETIME;
        p.velocity = initVelocity.x + asfloat(veloV)  * initVelocity.y;
        p.direction.x = initAlpha.x + asfloat(alphaV) * initAlpha.y;
        p.direction.y =  initBeta.x + asfloat(betaV)  * initBeta.y;
    }
   
    outputData[dispatchThreadID.x] = p;
    postParticles.Append(p);
}