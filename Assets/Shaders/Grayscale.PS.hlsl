#include "CpyImg.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material {
    float4 color;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelOutput{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexShaderOutput input) {
    PixelOutput output;
    output.color = gTexture.Sample(gSampler, input.texCoord);
    float value = dot(output.color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
    output.color.rgb = float3(value, value, value) * gMaterial.color.rgb;
    output.color.a = 1.f;
    return output;
};
