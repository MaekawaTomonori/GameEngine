#include "CpyImg.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material {
    float4 color;
    float intensity;
    float scale;
    float2 pad;
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
    value = saturate(pow(value, gMaterial.scale));
    float edge = 1.f - value;
    float strength = edge * gMaterial.color.a;
    output.color.rgb = lerp(output.color.rgb, gMaterial.color.rgb, strength);
    //output.color.rgb = lerp(gMaterial.color.rgb, output.color.rgb, value);
    output.color.a = 1.f;
    return output;
};
