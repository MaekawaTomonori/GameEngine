#include "Line.hlsli"

struct Material {
    float32_t4 color;
};
ConstantBuffer<Material> gMaterial : register(b0);

float4 main(VertexShaderOutput input) : SV_TARGET {
    return gMaterial.color;
}