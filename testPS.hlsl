Texture2D tex2d : register(t0);
SamplerState smplr : register(s3);

float4 main(float2 tc : TEXCOORD) : SV_Target
{
    float3 tmp = tex2d.Sample(smplr, tc).rgb;
    return float4(tmp, 1.0f);
}