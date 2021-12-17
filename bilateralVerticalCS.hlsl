
cbuffer cbuf : register(b2)
{
    uint radius;
    float rsigma;
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

    float4 centerColor = inputMap.Load(tc);
    
    float4 color = centerColor * kernelValue[0][0];
    float4 weight = kernelValue[0][0];

    for (int i = 1; i < (int) radius; i++)
    {
        float val = kernelValue[i>>2][i&3];
        
        for (int j = -1; j < 2; j+=2)
        {
            int3 tmp = tc + int3(0, j*i, 0);
            tmp.y = clamp(tmp.y, 0, screen_height);
            float4 sample1 = inputMap.Load(tmp);
            float4 delta = centerColor - sample1;
            //float4 range = exp(-(delta * delta) / (2.0f * 0.051f * 0.051f));
            float4 range = exp(-(delta * delta) / (2.0f * rsigma * rsigma));
            color += sample1 * range * kernelValue[i];
            weight += range * kernelValue[i];
        }
    }
       
    outputMap[DispatchThreadID.xy] = color / weight;
}