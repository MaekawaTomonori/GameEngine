#include "Particle.hlsli"

struct Particle{
    float32_t4x4 wvp;
    float32_t4x4 world;
    float32_t4 color;
}
StructuredBuffer<Particle> gParticles : register(t0);

struct VSInput{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
}

VSOutput main(VSInput input, uint instanceID : SV_InstanceID){
    VSOutput output;

    output.position = mul(input.wvp, gParticles[instanceID].wvp);
    output.uv = input.uv;
    output.color = gParticles[instanceID].color;

    return output;
}
