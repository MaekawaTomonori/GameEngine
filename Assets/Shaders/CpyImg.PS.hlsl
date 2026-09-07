#include "CpyImg.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelOutput{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexShaderOutput input) {
    PixelOutput output;
    float4 sampled = gTexture.Sample(gSampler, input.texCoord);

    // ストレートアルファのテクスチャを事前乗算アルファへ変換して出力する
    output.color = float4(sampled.rgb * sampled.a, sampled.a);
    return output;
};
