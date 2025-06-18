#include "Sprite.hlsli"

struct Material{
    float32_t4 color;
};
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;
    output.color = pow(gTexture.Sample(gSampler, input.texcoord) * gMaterial.color, 2.2);
    return output;
};