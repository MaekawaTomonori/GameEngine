#include "Text.hlsli"

// SDF アトラス（R8_UNORM）
Texture2D<float32_t> gAtlas   : register(t0);
SamplerState         gSampler : register(s0);

float32_t4 main(VSOutput input) : SV_Target {
    float32_t d = gAtlas.Sample(gSampler, input.uv);

    // SDF エッジ値（FontAtlas::kOnEdgeValue = 180 を正規化）
    const float32_t kEdge = 180.0f / 255.0f;

    // fwidth によるアンチエイリアシング幅を計算
    float32_t aa = fwidth(d) * 0.75f;
    float32_t alpha = smoothstep(kEdge - aa, kEdge + aa, d);

    if (alpha < 0.001f) {
        discard;
    }

    return float32_t4(input.color.rgb, input.color.a * alpha);
}
