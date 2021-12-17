
struct PS_INPUT
{
    float2 inTexCoord : TC;
};

Texture2D objTexture : register(t0);
SamplerState objSamplerState : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    return objTexture.Sample(objSamplerState, input.inTexCoord);
}