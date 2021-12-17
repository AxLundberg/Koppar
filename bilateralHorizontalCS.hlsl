cbuffer cbuf : register(b2)
{
    uint radius;
    float rsigma;
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

    float4 centerColor = inputMap.Load(tc);
    
    float4 color = centerColor * kernelValue[0][0];
    float4 weight = kernelValue[0][0];

    for (int i = 1; i < (int) radius; i++)
    {
        float val = kernelValue[i>>2][i&3];
        
        for (int j = -1; j < 2; j += 2)
        {
            int3 tmp = tc + int3(j * i, 0, 0);
            tmp.x = clamp(tmp.x, 0, screen_width);
            float4 sample1 = inputMap.Load(tmp);
            float4 delta = centerColor - sample1;
            //float4 range = exp(-1.0f * (delta * delta) / (2.0f * 0.051f * 0.051f));
            float4 colorRange = exp(-1.0f * (delta * delta) / (2.0f * rsigma * rsigma));
            color += sample1 * colorRange * kernelValue[i];
            weight += colorRange * kernelValue[i];
        }
    }
       
    outputMap[DispatchThreadID.xy] = color / weight;
}