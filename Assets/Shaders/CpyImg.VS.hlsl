#include "CpyImg.hlsli"

static const uint32_t VERTEX_COUNT = 3;

static const float4 POSITIONS[VERTEX_COUNT] = {
    {-1.f, 1.f, 0.f, 1.f},
    {3.f, 1.f, 0.f, 1.f},
    {-1.f, -3.f, 0.f, 1.f}
};

static const float2 TEXCOORDS[VERTEX_COUNT] = {
    {0.f, 0.f},
    {2.f, 0.f},
    {0.f, 2.f}
};

VertexShaderOutput main(uint vertexId : SV_VertexID) {
    VertexShaderOutput output;
    output.position = POSITIONS[vertexId];
    output.texCoord = TEXCOORDS[vertexId];
    return output;
};
