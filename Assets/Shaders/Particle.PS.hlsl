#include "Particle.hlsli"

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PSOutput {
    float32_t4 color : SV_Target0;
};

PSOutput main(VSOutput input) {
    PSOutput output;

    float32_t4 texColor = gTexture.Sample(gSampler, input.uv);

    if(texColor.a == 0.0f){
        discard;
    }

    output.color = texColor * input.color;

    if(output.color.a < 0.001f){
        discard;
    }

    return output;
}
