struct GSOutput
{
    float3 view_pos : POSITION;
    float4 shadow_pos : SHADOWPOSITION;
    float3 normal : NORMAL;
    float2 tc : TEXCOORD;
    float3x3 modelview : MATRIX;
    float4 pos : SV_POSITION;
};

[maxvertexcount(3)]
void main(
	triangle GSOutput input[3], 
	inout TriangleStream< GSOutput > output
)
{
    float3 v1 = input[1].view_pos - input[0].view_pos;
    float3 v2 = input[2].view_pos - input[0].view_pos;
    float3 n = normalize(cross(v1, v2));
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.view_pos = input[i].view_pos;
        element.normal = n;
        element.shadow_pos = input[i].shadow_pos;
        element.pos = input[i].pos;
        element.tc = input[i].tc;
        element.modelview = input[i].modelview;
		output.Append(element);
	}
}