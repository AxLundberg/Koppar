
cbuffer cbuf : register(b2)
{
    uint radius;
    float4 kernelValue[16];
};

Texture2D<float4> inputMap : register(t0);
RWTexture2D<float4> outputMap : register(u0);

#define screen_width 1899
#define size_x 950
#define size_y 1 


[numthreads(size_x, 1, 1)]
void main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    int3 tc = DispatchThreadID;
    
    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    
    color += inputMap.Load(tc) * kernelValue[0][0];
    
    for (int i = 1; i <= (int) radius; i++)
    {
        float val = kernelValue[i >> 2][i & 3];
        int3 tc1 = tc + int3(i, 0, 0);
        int3 tc2 = tc - int3(i, 0, 0);
        tc1.x = clamp(tc1.x, 0, screen_width);
        tc2.x = clamp(tc2.x, 0, screen_width);
        color += (inputMap.Load(tc2) + inputMap.Load(tc1)) * val;
    }
    
    outputMap[DispatchThreadID.xy] = color;
}