struct PSInput {
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
};

struct ShadowCamera {
    float4x4 faceVP;
    float3   lightPos;
    float    farPlane;
};
ConstantBuffer<ShadowCamera> gShadowCamera : register(b1);

float main(PSInput input) : SV_TARGET {
    return length(input.worldPos - gShadowCamera.lightPos) / gShadowCamera.farPlane;
}
