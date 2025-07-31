#include "Sky.hlsli"

struct Material{
    float32_t4 color;
};
ConstantBuffer<Material> gMaterial : register(b0);

TextureCube<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 texColor = gTexture.Sample(gSampler, normalize(input.texcoord));
    output.color = texColor * gMaterial.color;
    return output;
}
