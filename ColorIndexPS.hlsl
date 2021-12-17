
cbuffer cbuf : register(b1)
{
	float4 face_colors;
};

float4 main(uint tid : SV_PrimitiveID) : SV_TARGET
{
    //return float4(saturate(float3(0.99f, 0.8f, 0.3f)), 1.0f);
    return face_colors;
}