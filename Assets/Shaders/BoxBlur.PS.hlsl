#include "CpyImg.hlsli"

static const float2 INDICES[3][3] = {
    { {-1.f, -1.f}, {0.f, -1.f}, {1.f, -1.f} },
    { {-1.f, 0.f}, {0.f, 0.f}, {1.f, 0.f} },
    { {-1.f, 1.f}, {0.f, 1.f}, {1.f, 1.f} }
};

static const float KERNELS[3][3] = {
    { 1.f / 9.f, 1.f / 9.f, 1.f / 9.f },
    { 1.f / 9.f, 1.f / 9.f, 1.f / 9.f },
    { 1.f / 9.f, 1.f / 9.f, 1.f / 9.f }
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material {
    float4 color;
    float radius;
    float strength;
    float2 pad;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelOutput{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexShaderOutput input) {
    uint32_t width, height;
    gTexture.GetDimensions(width, height);
    float2 uvSize = {rcp(width), rcp(height)};

    float4 original = gTexture.Sample(gSampler, input.texCoord);

    float4 blurred = float4(0.f, 0.f, 0.f, 0.f);
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            float2 uv = input.texCoord + INDICES[x][y] * uvSize * gMaterial.radius;
            blurred += gTexture.Sample(gSampler, uv) * KERNELS[x][y];
        }
    }

    PixelOutput output;
    output.color = lerp(original, blurred, saturate(gMaterial.strength));
    output.color.rgb *= gMaterial.color.rgb;
    return output;
};
