struct VSOutput {
    float32_t4 position : SV_Position;
    float32_t2 uv : TEXCOORD0;
    float32_t4 color: COLOR0;
};

//struct PreView{
//    float32_t4x4 viewProjection;
//    float32_t4x4 billboardMatrix;
//}
//
//struct Particle {
//    float32_t3 translate;
//    float32_t3 scale;
//    float32_t lifetime;
//    float32_t3 velocity;
//    float32_t currentTime;
//    float32_t4 color;
//}
//
//static const uint32_t MAX = 1024;
