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

SamplerState shadowMapSmplr : register(s1);
Texture2D shadowMap : register(t2);

float4 main(float3 view_pos: POSITION, float4 shadowPos: SHADOWPOSITION, float3 normal : NORMAL) : SV_TARGET
{
    float3 diffused_color = float3(0.0f, 0.0f, 0.0f);
    
    float sMapDepth = shadowMap.Sample(shadowMapSmplr, shadowPos.xy).r;
    
    if (sMapDepth < 0.01f || sMapDepth > shadowPos.z - 0.005f) // We baskin (or !shadowPass)
    {
        float3 vec_to_light = light_pos - view_pos;
        float dist_to_light = sqrt(vec_to_light.x * vec_to_light.x +
							   vec_to_light.y * vec_to_light.y +
							   vec_to_light.z * vec_to_light.z);
	
        float3 direction_to_light = vec_to_light / dist_to_light;
        
        float attenuation = attenuationConst +
                            attenuationLinear * dist_to_light +
                            attenuationQuad * (dist_to_light * dist_to_light); //attenuation algoritm (OGRE wiki)
 
        float luminosity = 1.0f / attenuation;
        
        diffused_color = diffuseColor * diffuseIntensity * luminosity *
			    	 max(0, dot(direction_to_light, normal));
    }
    
    return float4(saturate((diffused_color + ambient)), 1.0f);
}