cbuffer LightCBuffer
{
    float4 light_pos;
    float4 ambient;
    float4 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer ObjectCBuf
{
    float4 objDiffuseColor;
    float4 objSpecColor;
    float specularIntensity;
    float specularPower;
    bool hasNormalMap;
    bool hasDiffuseTexture;
};

struct PS_INPUT
{
    float3 viewpos   : POSITION;
    float4 shadowPos : SHADOWPOSITION;
    float3 normal    : NORMAL;
    float2 tc        : TEXCOORD;
    float3x3 TBN     : MATRIX;
    float4 pos       : SV_POSITION;
};

SamplerState objSmplr : register(s0);
SamplerState shadowMapSmplr : register(s1);
SamplerComparisonState smapSmplr : register(s2);

Texture2D diffuseTexture : register(t0);
Texture2D normalMap : register(t1);
Texture2D shadowMap : register(t2);

#define SAMPLE_RANGE 2
#define N_SAMPLES ((SAMPLE_RANGE*2+1) * (SAMPLE_RANGE*2+1))

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 objDiffColor;
    float3 normal;
    
    if(hasNormalMap)
    {
        const float3 sampleNormal = normalMap.Sample(objSmplr, input.tc).rgb;
        normal = sampleNormal * 2.0f - 1.0f;
        normal = normalize(mul(normal, input.TBN));
    }
    else
    {
        normal = normalize(input.normal);
    }
    
    if(hasDiffuseTexture)
    {
        objDiffColor = diffuseTexture.Sample(objSmplr, input.tc).rgb;
    }
    else
    {
        objDiffColor = objDiffuseColor.xyz;
    }
    
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    
    float lightFactor = 0.0f;
    const float3 s_pos  = input.shadowPos.xyz / input.shadowPos.w;
    const float smDepth = shadowMap.Sample(shadowMapSmplr, float2(s_pos.xy)).r;
    //full light for outside lightproj OR if no shadowpass
    if (s_pos.z > 1.0f || smDepth == 0.0f)
    {
        lightFactor = 1.0f;
    }
    else // calculate lightfactor
    {
        [unroll]
        for (int u = -SAMPLE_RANGE; u <= SAMPLE_RANGE; u++)
        {
            [unroll]
            for (int v = -SAMPLE_RANGE; v <= SAMPLE_RANGE; v++)
            {
                lightFactor += shadowMap.SampleCmpLevelZero(smapSmplr, s_pos.xy, s_pos.z - 0.00005f, int2(u, v)); 
            }
        }
        lightFactor = lightFactor / N_SAMPLES;
    }
    float3 vec_to_light = light_pos.xyz - input.viewpos;
    
    float dist_to_light = sqrt(vec_to_light.x * vec_to_light.x +
							   vec_to_light.y * vec_to_light.y +
							   vec_to_light.z * vec_to_light.z);
    
    float3 direction_to_light = vec_to_light / dist_to_light;
    
    float attenuation = attConst +
                        attLin * dist_to_light +
                        attQuad * (dist_to_light * dist_to_light); //attenuation algoritm (OGRE wiki)
    
    float luminosity = 1.0f / attenuation;
    
    float3 diffused_color = diffuseColor.rgb * diffuseIntensity * luminosity *
					        max(0, dot(direction_to_light, normal));
        
    // reflected light vector
    const float3 w = normal * dot(vec_to_light, normal);
    const float3 r = w * 2.0f - vec_to_light;
	// calculate specular based on angle between viewing and reflection vector, narrowed with power function
    specular = luminosity *
               diffuseColor.rgb * diffuseIntensity * specularIntensity *
               pow(max(0.0f, dot(normalize(-r), normalize(input.viewpos))), specularPower);
    
    return float4(saturate((lightFactor * diffused_color + ambient.rgb) * objDiffColor + objSpecColor.rgb * (lightFactor * specular)), 1.0f);
    //return float4(normalRGB, 1.0f);
}
