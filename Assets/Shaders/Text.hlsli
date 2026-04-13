// Text.hlsli - テキスト描画共有構造体

struct VSOutput {
    float32_t4 position : SV_Position;
    float32_t2 uv       : TEXCOORD0;
    float32_t4 color    : COLOR0;
};
