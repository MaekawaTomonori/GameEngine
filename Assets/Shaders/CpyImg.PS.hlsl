#include "CpyImg.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelOutput{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexShaderOutput input) {
    PixelOutput output;
    output.color = gTexture.Sample(gSampler, input.texCoord);
    return output;
};
