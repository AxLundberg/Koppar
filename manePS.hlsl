cbuffer LightCBuffer
{
    float3 light_pos;
    float padd1;
    float3 ambient;
    float padd2;
    float3 diffuseColor;
    float padd3;
    float diffuseIntensity;
    float attenuationConst;
    float attenuationLinear;
    float attenuationQuad;
};

struct PS_INPUT
{
    float3 world_pos  : POSITION;
    float3 normal     : NORMAL;
    float4 shadow_pos : SHADOWPOSITION;
    float2 tc         : TEXCOORD;
    float3x3 TBN      : MATRIX;
    float4 pos        : SV_POSITION;
};

Texture2D normalMap : register(t1); 
Texture2D shadowMap : register(t2);
SamplerState objSmplr : register(s0);
SamplerState shadowMapSmplr : register(s1);

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 vec_to_light = light_pos - input.world_pos;
    float dist_to_light = sqrt(vec_to_light.x * vec_to_light.x +
							   vec_to_light.y * vec_to_light.y +
							   vec_to_light.z * vec_to_light.z);
	
    float3 direction_to_light = vec_to_light / dist_to_light;
    float attenuation = 1.0f / (attenuationConst + attenuationLinear * dist_to_light + attenuationQuad * (dist_to_light * dist_to_light)); //attenuation algoritm (OGRE wiki)
    //attenuation = 1.0f;
    float3 normalRGB = (float3)normalMap.Sample(objSmplr, input.tc);
    float3 normal = normalRGB * 2.0f - 1.0f;
    normal = normalize(mul(normal, input.TBN));
    //normal = input.normal;
    //normalize(normal);
    
    float3 diffused_color = diffuseColor * diffuseIntensity * attenuation *
							max(0, dot(direction_to_light, normal));
	
    //return float4(saturate(diffused_color + ambient), 1.0f);
    return float4(normalRGB, 1.0f);
}
