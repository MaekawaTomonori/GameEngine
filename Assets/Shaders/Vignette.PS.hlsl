#include "CpyImg.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material {
    float4 color;
    float intensity;
    float3 pad;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelOutput{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexShaderOutput input) {
    PixelOutput output;
    output.color = gTexture.Sample(gSampler, input.texCoord);
    float2 correct = input.texCoord * (1.f - input.texCoord.yx);
    float value = correct.x * correct.y * gMaterial.intensity;
    value = saturate(pow(value, 0.8f));
    output.color.rgb *= value * gMaterial.color.rgb;
    output.color.a = 1.f;
    return output;
};
