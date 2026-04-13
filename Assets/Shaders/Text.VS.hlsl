#include "Text.hlsli"

// グリフ1文字分のインスタンスデータ（StructuredBuffer 経由）
struct GlyphInstance {
    float32_t2 position; // 画面空間での左上位置（ピクセル）
    float32_t2 size;     // 画面空間でのサイズ（ピクセル）
    float32_t2 uvPos;    // アトラス UV 左上（0-1）
    float32_t2 uvSize;   // アトラス UV サイズ（0-1）
    float32_t4 color;    // RGBA
};

StructuredBuffer<GlyphInstance> gGlyphs : register(t0);

cbuffer ScreenParams : register(b0) {
    float32_t2 screenSize;
    float32_t2 pad;
};

// 1クワッドを 6 頂点（三角形 2 枚）で構成するコーナーオフセット
static const float32_t2 kCorner[6] = {
    float32_t2(0, 0), float32_t2(1, 0), float32_t2(0, 1),
    float32_t2(1, 0), float32_t2(1, 1), float32_t2(0, 1)
};

VSOutput main(uint32_t vertexID : SV_VertexID, uint32_t instanceID : SV_InstanceID) {
    GlyphInstance g = gGlyphs[instanceID];
    float32_t2 corner = kCorner[vertexID];

    // ピクセル座標を算出
    float32_t2 pixelPos = g.position + corner * g.size;

    // ピクセル座標 → クリップ座標（Y 軸反転）
    float32_t2 clip;
    clip.x =  (pixelPos.x / screenSize.x) * 2.0f - 1.0f;
    clip.y = -((pixelPos.y / screenSize.y) * 2.0f - 1.0f);

    VSOutput output;
    output.position = float32_t4(clip, 0.0f, 1.0f);
    output.uv       = g.uvPos + corner * g.uvSize;
    output.color    = g.color;
    return output;
}
