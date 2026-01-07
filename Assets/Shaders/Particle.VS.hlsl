#include "Particle.hlsli"

struct Particle{
    float32_t4x4 wvp;
    float32_t4x4 world;
    float32_t4 color;
};
StructuredBuffer<Particle> gParticles : register(t0);

struct VSInput{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
};

VSOutput main(VSInput input, uint instanceID : SV_InstanceID){
    VSOutput output;

    output.position = mul(input.position, gParticles[instanceID].wvp);
    output.uv = input.uv;
    output.color = gParticles[instanceID].color;

    return output;
}
