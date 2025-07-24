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
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelOutput{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexShaderOutput input) {
    uint32_t width, height;
    gTexture.GetDimensions(width, height);
    float2 uvSize = {rcp(width), rcp(height)};

    PixelOutput output;
    output.color = gTexture.Sample(gSampler, input.texCoord);

    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            float2 uv = input.texCoord + INDICES[x][y] * uvSize;
            float3 color = gTexture.Sample(gSampler, uv).rgb;
            output.color.rgb += color * KERNELS[x][y];
        }
    }

    output.color.a = 1.0f;
    return output;
};
