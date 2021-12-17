Texture2D tex2d : register(t2);
SamplerState splr : register(s0);


float4 main(float2 uv : TEXCOORD) : SV_Target
{
    float color = tex2d.Sample(splr, uv).r;
    color = 0.01f / (1.01f - color);
    float4 c = { color, color, color, 1.0f };
    return c;
}