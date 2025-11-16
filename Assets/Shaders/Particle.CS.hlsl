#include "Particle.hlsli"

RWStructuredBuffer<Particle> gParticles : register(u0);
[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    uint32_t id = DTid.x;
    if(id < MAX){
        gParticles[id] = (Particle)0;
        gParticles[id].scale = float32_t3(0.5f, 0.5f, 0.5f);
        gParticles[id].color = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
     }
}