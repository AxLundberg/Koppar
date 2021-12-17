
Texture2D shadowMap : register(t2);
SamplerState splr : register(s1);
SamplerComparisonState smapsplr : register(s2);

float4 main(float3 view_pos : POSITION, float4 shadowPos : SHADOWPOSITION) : SV_TARGET
{
    shadowPos.xyz = shadowPos.xyz / shadowPos.w;
    //if (shadowMap.Sample(splr, shadowPos.xy).r > shadowPos.z - 0.0001f || shadowPos.z >= 0.99999f || shadowPos.z < 0.0001f) // We baskin
    //{
        if (shadowPos.z >= 0.99999f || shadowPos.z < 0.0001f) //full light if outside light vision
        {
            return float4(0.5f, 0.5f, 0.5f, 1.0f);
        }
        
        //float factor = 0.1f;
        float factor = 0.0f;
        [unroll]
        for (int u = -2; u <= 2; u++)
        {
            [unroll]
            for (int v = -2; v <= 2; v++)
            {
                //const float smpleDepth = shadowMap.Sample(splr, shadowPos.xy, int2(u,v)).r;
                //factor += smpleDepth > shadowPos.z - 0.0001f ? 0.1f : 0.0f;
                factor += shadowMap.SampleCmpLevelZero(smapsplr, shadowPos.xy, shadowPos.z - 0.0001f, int2(u, v));

            }
        }
        factor /= 25;
        
        return float4(factor * 0.5f, factor * 0.5f, factor * 0.5f, 1.0f);

    //}
    //else
    //{
    //    return float4(0.0f, 0.0f, 0.0f, 1.0f); // We ain't baskin
    //}
}