struct PSInput {
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float2 texcoord : TEXCOORD1;
};

struct ShadowCamera {
    float4x4 faceVP;
    float3   lightPos;
    float    farPlane;
};
ConstantBuffer<ShadowCamera> gShadowCamera : register(b1);

struct ShadowMaterial {
    float4 color;
};
ConstantBuffer<ShadowMaterial> gMaterial : register(b0);

Texture2D    gTexture : register(t0);
SamplerState gSampler : register(s0);

float main(PSInput input) : SV_TARGET {
    float texAlpha      = gTexture.Sample(gSampler, input.texcoord).a;
    float combinedAlpha = gMaterial.color.a * texAlpha;

    // Fully transparent: cast no shadow
    clip(combinedAlpha - 0.01f);

    // Semi-transparent: thin the shadow stochastically
    if (combinedAlpha < 0.99f) {
        float noise = frac(sin(dot(input.worldPos.xz, float2(127.1f, 311.7f))) * 43758.5453f);
        clip(combinedAlpha - noise);
    }

    return length(input.worldPos - gShadowCamera.lightPos) / gShadowCamera.farPlane;
}
