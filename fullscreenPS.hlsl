Texture2D tex2d : register(t2);
SamplerState splr : register(s0);

float4 main(float2 uv : Texcoord) : SV_Target
{
    return tex2d.Sample(splr, uv).rgba;
}