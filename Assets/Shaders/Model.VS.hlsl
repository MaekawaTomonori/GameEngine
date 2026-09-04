#include "Model.hlsli"

struct TransformationMatrix{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4x4 WorldInverseTranspose;
};
StructuredBuffer<TransformationMatrix> gTransformationMatrix : register(t2);

struct VertexShaderInput{
	float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID){
    VertexShaderOutput output;
    TransformationMatrix transform = gTransformationMatrix[instanceId];

    output.position = mul(input.position, transform.WVP);
    output.texcoord = input.texcoord;

    //Lambertian Reflectance
    output.normal = normalize(mul(input.normal, (float32_t3x3)transform.WorldInverseTranspose));

    //Phong Reflection Model
    output.worldPosition = mul(input.position, transform.World).xyz;

    return output;
}
