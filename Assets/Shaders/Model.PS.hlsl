#include "Model.hlsli"

struct Material{
    float32_t4 color;
    uint32_t enableLighting;
    float32_t shininess;
    float coefficient;
    float2 tilingMul;
    float4x4 uvTransform;
};
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct DirectionalLight{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};
StructuredBuffer<DirectionalLight> gDirectionalLight : register(t1);

struct Camera{
    float32_t3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b2);

struct PointLight{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float2 pad;
};
StructuredBuffer<PointLight> gPointLight : register(t3);

struct SpotLight{
    float32_t4 color;
    float32_t3 position;
    float32_t intensity;
    float32_t3 direction;
    float32_t distance;
    float32_t decay;
    float32_t cosAngle;
    float32_t cosFalloffStart;
    float pad;
};
StructuredBuffer<SpotLight> gSpotLight : register(t4);

struct LightCount{
    uint dlCount;
    uint plCount;
    uint slCount;
};
ConstantBuffer<LightCount> gLightCount : register(b5);

TextureCube<float32_t4> gEnvironment : register(t5);

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 transformedUV = mul(float4(input.texcoord, 0.f, 1.f), gMaterial.uvTransform);
    transformedUV.xy *= gMaterial.tilingMul;

    float32_t4 texColor = gTexture.Sample(gSampler, transformedUV.xy);

    if (texColor.a == 0.f)
    {
        discard;
    }

    float32_t3 rgb = gMaterial.color.rgb * texColor.rgb;
    float32_t3 finalRGB = rgb;
    float32_t a = gMaterial.color.a * texColor.a;

    //if disable lighting
    if (gMaterial.enableLighting == 0) {
        output.color = float32_t4(rgb, a);
        return output;
    }

    float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float3 halfVector = normalize(toEye + input.normal);

    float3 col = float3(0,0,0);
    for (uint i = 0; i < gLightCount.dlCount; ++i) {
	//directional
        float nDotL = dot(normalize(input.normal), -gDirectionalLight[i].direction);
        float cos = pow(nDotL * 0.5f + 0.5f, 2.0f);
	    
        float32_t3 reflectLight = reflect(gDirectionalLight[i].direction, normalize(input.normal));

		halfVector = normalize(-gDirectionalLight[i].direction + toEye);
        float nDotH = saturate(dot(normalize(input.normal), halfVector));
        float specularPow = pow(saturate(nDotH), gMaterial.shininess);

        float32_t3 diffuse = rgb * gDirectionalLight[i].color.rgb * cos * gDirectionalLight[i].intensity;
        float32_t3 specular = gDirectionalLight[i].color.rgb * gDirectionalLight[i].intensity * specularPow * float32_t3(1.f, 1.f, 1.f);
        col += diffuse + specular;
    }

    for (uint j = 0; j < gLightCount.plCount; ++j) {
    //point
        float32_t3 pointDirection = normalize(input.worldPosition - gPointLight[j].position);

        float32_t distance = length(gPointLight[j].position - input.worldPosition);
        float32_t factor = pow(saturate(-distance / gPointLight[j].radius + 1.0), gPointLight[j].decay);

        float cosP = pow(dot(normalize(input.normal), -pointDirection) * 0.5f + 0.5f, 2.0f);

        float32_t3 halfVectorP = normalize(-pointDirection + toEye);
        float specularPowP = pow(saturate(dot(normalize(input.normal), halfVector)), gMaterial.shininess);

        float32_t3 pointDiffuse = rgb * gPointLight[j].color.rgb * cosP * gPointLight[j].intensity * factor;
        float32_t3 pointSpecular = gPointLight[j].color.rgb * gPointLight[j].intensity * factor * specularPowP * float32_t3(1.f, 1.f, 1.f);

        col += pointDiffuse + pointSpecular;
    }

    for (uint k = 0; k < gLightCount.slCount; ++k) {
    //spot
        float32_t3 spotDirection = normalize(input.worldPosition - gSpotLight[k].position);
        float32_t cosAngle = dot(spotDirection, gSpotLight[k].direction);
        float32_t falloffFactor = saturate((cosAngle - gSpotLight[k].cosAngle) / (gSpotLight[k].cosFalloffStart - gSpotLight[k].cosAngle));
        float32_t distance = length(gSpotLight[k].position - input.worldPosition);

        float32_t attenuationFactor = 1.f / (1.f + gSpotLight[k].decay * pow(distance / gSpotLight[k].distance, 2.f));

        col += rgb * gSpotLight[k].color.rgb * gSpotLight[k].intensity * attenuationFactor * falloffFactor;
    }

    finalRGB = col;

    // Environment mapping (only if coefficient > 0)
    if (gMaterial.coefficient > 0.0f) {
        float3 ctp = normalize(input.worldPosition - gCamera.worldPosition);
        float3 reflectDir = reflect(ctp, normalize(input.normal));
        float4 environmentColor = gEnvironment.Sample(gSampler, reflectDir);
        finalRGB += environmentColor.rgb * gMaterial.coefficient;
    }

    //final set
    output.color = float32_t4(finalRGB, a);

    if (output.color.a == 0.f){discard;}

    return output;
}
