
cbuffer cbuf : register(b2)
{
    uint radius;
    float4 kernelValue[16];
};

Texture2D<float4> inputMap : register(t0);
RWTexture2D<float4> outputMap : register(u0);

#define size_x 1
#define size_y 500
#define screen_height 999

[numthreads(1, size_y, 1)]
void main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    int3 tc = DispatchThreadID;

    float4 color = float4(0.0, 0.0, 0.0, 0.0);

    color += inputMap.Load(tc) * kernelValue[0][0];
    for (int i = 1; i <= (int) radius; i++)
    {
        float val = kernelValue[i>>2][i&3];
        int3 tmp1 = tc + int3(0, i, 0);
        int3 tmp2 = tc - int3(0, i, 0);
        tmp1.y = clamp(tmp1.y, 0, screen_height);
        tmp2.y = clamp(tmp2.y, 0, screen_height);
        color += inputMap.Load(tmp1) * val;
        color += inputMap.Load(tmp2) * val;
    }
       
    outputMap[DispatchThreadID.xy] = color;
}