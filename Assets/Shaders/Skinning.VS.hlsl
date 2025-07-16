#include "Model.hlsli"

struct TransformationMatrix{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4x4 WorldInverseTranspose;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct Well{
    float32_t4x4 space;
    float32_t4x4 inverseTranspose;
};
StructuredBuffer<Well> gMatrixPalette : register(t0);

struct VertexShaderInput{
	float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t4 weight : WEIGHT0;
    int32_t4 index : INDEX0;
};

struct Skinned{
    float32_t4 position;
    float32_t3 normal;
};

Skinned Skinning(VertexShaderInput input){
    Skinned skinned;
    
    skinned.position  = mul(input.position, gMatrixPalette[input.index.x].space) * input.weight.x;
    skinned.position += mul(input.position, gMatrixPalette[input.index.y].space) * input.weight.y;
    skinned.position += mul(input.position, gMatrixPalette[input.index.z].space) * input.weight.z;
    skinned.position += mul(input.position, gMatrixPalette[input.index.w].space) * input.weight.w;
    skinned.position.w = 1.0f;

    skinned.normal  = mul(input.normal, (float32_t3x3)gMatrixPalette[input.index.x].inverseTranspose) * input.weight.x;
    skinned.normal += mul(input.normal, (float32_t3x3)gMatrixPalette[input.index.y].inverseTranspose) * input.weight.y;
    skinned.normal += mul(input.normal, (float32_t3x3)gMatrixPalette[input.index.z].inverseTranspose) * input.weight.z;
    skinned.normal += mul(input.normal, (float32_t3x3)gMatrixPalette[input.index.w].inverseTranspose) * input.weight.w;
    skinned.normal = normalize(skinned.normal);

    return skinned;
}

VertexShaderOutput main(VertexShaderInput input){
    VertexShaderOutput output;
    Skinned skinned = Skinning(input);
    output.position = mul(skinned.position, gTransformationMatrix.WVP);
    output.worldPosition = mul(skinned.position, gTransformationMatrix.World).xyz;
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3)gTransformationMatrix.WorldInverseTranspose));

    return output;
}

