struct VSInput {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal   : NORMAL;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
};

struct Transformation {
    float4x4 wvp;
    float4x4 world;
    float4x4 inverseTranspose;
};
ConstantBuffer<Transformation> gTransformation : register(b0);

struct ShadowCamera {
    float4x4 faceVP;
    float3   lightPos;
    float    farPlane;
};
ConstantBuffer<ShadowCamera> gShadowCamera : register(b1);

VSOutput main(VSInput input) {
    VSOutput output;
    float4 worldPos  = mul(input.position, gTransformation.world);
    output.worldPos  = worldPos.xyz;
    output.position  = mul(worldPos, gShadowCamera.faceVP);
    return output;
}
