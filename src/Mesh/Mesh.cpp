#include "Mesh.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>

#include "Utils.hpp"
#include "DebugUI.hpp"
#include "Singleton.hpp"
#include "imgui.h"
#include "Math/Vector3.hpp"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "src/Light/LightManager.hpp"
#include "src/Texture/TextureManager.hpp"

void Mesh::Initialize(DirectXAdapter* _adapter, const std::string &_directory, const std::string &_name) {
    adapter_ = _adapter;
    commandList_ = adapter_->GetCommandList();
    name_ = _name;

    LoadFile(_directory, _name);

    vr_.Attach(adapter_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size()));

    vbv_.BufferLocation = vr_->GetGPUVirtualAddress();
    vbv_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
    vbv_.StrideInBytes = sizeof(VertexData);

    vr_->Map(0, nullptr, reinterpret_cast<void**>(&vd_));
    std::copy_n(modelData_.vertices.data(), modelData_.vertices.size(), vd_);

    mr_.Attach(adapter_->CreateBufferResource(sizeof(Material)));
    mr_->Map(0, nullptr, reinterpret_cast<void**>(&material_));

    material_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    material_->lighting = 0; // Default lighting
    material_->shininess = 100.f;

    Singleton<TextureManager>::GetInstance()->Load(modelData_.material.texture);
    texture_ = modelData_.material.texture;
}

void Mesh::Update() {
}

void Mesh::Draw() {
    if (!commandList_)return;

    commandList_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList_->IASetVertexBuffers(0, 1, &vbv_);
	commandList_->SetGraphicsRootConstantBufferView(0, mr_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootDescriptorTable(2, Singleton<TextureManager>::GetInstance()->GetGPUHandle(texture_));

    if (lighting_) {
        Singleton<LightManager>::GetInstance()->Draw();
    }

    commandList_->DrawInstanced(static_cast<UINT>(modelData_.vertices.size()), 1, 0, 0);
}

void Mesh::Debug() {
    ImGui::ColorEdit4("Color", &material_->color.x);
    ImGui::Checkbox("EnableLighting", &lighting_);
    if (lighting_) {
        material_->lighting = 1;
        ImGui::DragFloat("Shininess", &material_->shininess, 0.1f, 0.f, 100.f);
    } else{
        material_->lighting = 0;
    }
}

void Mesh::LoadFile(const std::string &_directory, const std::string &_name) {
    std::filesystem::path directory(_directory + _name);
    //obj
    if (exists(directory / (_name +".obj"))){
        LoadObj(Utils::Convert(directory), _name);
        return;
    }
    //gltf
    if (exists(directory / (_name + ".gltf"))) {
        LoadGltf(Utils::Convert(directory), _name);
        return;
    }

    Utils::Alert("Mesh::LoadFile: No valid mesh file found in directory: " + _directory);
}

void Mesh::LoadObj(const std::string& _directory, const std::string &_name) {
    Assimp::Importer importer;
    std::string path = _directory + "/" + _name + ".obj";
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    if (!scene->HasMeshes()){
        Utils::Alert("Mesh::LoadObj: No meshes found in file: " + path);
        return;
    }
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
    
        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];
    
            for (uint32_t element = 0; element < face.mNumIndices; ++element) {
                uint32_t vertex = face.mIndices[element];
                aiVector3D& position = mesh->mVertices[vertex];
                aiVector3D& normal = mesh->mNormals[vertex];
                aiVector3D texcoord = mesh->mTextureCoords[0][vertex];
    
                VertexData vertexData{};
                vertexData.position = Vector4(position.x, position.y, position.z, 1.0f);
                vertexData.texcoord = Vector2(texcoord.x, texcoord.y);
                vertexData.normal = Vector3(normal.x, normal.y, normal.z);
    
                vertexData.position.x *= -1; // Flip X axis
                vertexData.normal.x *= -1; // Flip X axis
    
                modelData_.vertices.push_back(vertexData);
            }
    
            for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
                    aiString texturePath;
                    material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
                    modelData_.material.texture = _directory + "/" + texturePath.C_Str();
                }
            }
        }
    }
}

void Mesh::LoadGltf(const std::string& _directory, const std::string& _name) {
    Assimp::Importer importer;
    std::string path = _directory + "/" + _name + ".gltf";
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    if (!scene->HasMeshes()){
        Utils::Alert("Mesh::LoadGltf: No meshes found in file: " + path);
        return;
    }

    modelData_.root = LoadNode(scene->mRootNode);
}

Node Mesh::LoadNode(const aiNode* _node) {
    Node result;
    aiMatrix4x4 local = _node->mTransformation;
    result.local = Matrix4x4{
        local.a1, local.b1, local.c1, local.d1,
        local.a2, local.b2, local.c2, local.d2,
        local.a3, local.b3, local.c3, local.d3,
        local.a4, local.b4, local.c4, local.d4
    };

    result.name = _node->mName.C_Str();
    result.children.reserve(_node->mNumChildren);
    for (uint32_t i = 0; i < _node->mNumChildren; ++i){
        result.children[i] = LoadNode(_node->mChildren[i]);
    }

    return result;
}

void Mesh::LoadAnimation(const std::string& _directory, const std::string& _name) {
    Assimp::Importer importer;
    std::string path = _directory + "/" + _name + ".gltf";
    const aiScene* scene = importer.ReadFile(path.c_str(), 0);

    aiAnimation* animation = scene->mAnimations[0];
    animation_.duration = static_cast<float>(animation->mDuration);

    for (uint32_t channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
        aiNodeAnim* nodeAnim = animation->mChannels[channelIndex];
        NodeAnimation& nodeAnimation = animation_.nodeAnimations[nodeAnim->mNodeName.C_Str()];
        for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumPositionKeys; ++keyIndex){
            aiVectorKey& positionKey = nodeAnim->mPositionKeys[keyIndex];
            nodeAnimation.translate.keyframes.push_back({ Vector3(-positionKey.mValue.x, positionKey.mValue.y, positionKey.mValue.z), static_cast<float>(positionKey.mTime) });
        }
        for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumRotationKeys; ++keyIndex){
            aiQuatKey& rotationKey = nodeAnim->mRotationKeys[keyIndex];
            nodeAnimation.rotation.keyframes.push_back({ Quaternion(rotationKey.mValue.x, -rotationKey.mValue.y, -rotationKey.mValue.z, rotationKey.mValue.w), static_cast<float>(rotationKey.mTime) });
        }
        for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumScalingKeys; ++keyIndex){
            aiVectorKey& scalingKey = nodeAnim->mScalingKeys[keyIndex];
            nodeAnimation.scale.keyframes.push_back({ Vector3(scalingKey.mValue.x, scalingKey.mValue.y, scalingKey.mValue.z), static_cast<float>(scalingKey.mTime) });
        }

        animation_.nodeAnimations[nodeAnim->mNodeName.C_Str()] = nodeAnimation;
    }
}


Mesh::MaterialData Mesh::LoadMaterialTemplateFile(std::string &_directory, std::string &_name) {
    MaterialData materialData {};
    std::string line;
    std::ifstream file(_directory + "/" + _name);
    assert(file.is_open());
    while (std::getline(file, line)){
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "map_Kd"){
            std::string textureFileName;
            s >> textureFileName;

            materialData.texture = _directory + "/" + textureFileName;
        }
    }
    return materialData;
}
