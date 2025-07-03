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
    if (std::filesystem::exists(directory / (_name +".obj"))){
        LoadObj(Utils::Convert(directory), _name);
        return;
    }
    //gltf
    if (std::filesystem::exists(directory / (_name + ".gltf"))) {
        //LoadGltf((file/".gltf").c_str());
        return;
    }

    Utils::Alert("Mesh::LoadFile: No valid mesh file found in directory: " + _directory);
}

void Mesh::LoadObj(const std::string& _directory, const std::string &_name) {
    //ModelData modelData {};
    //std::vector<Vector4> positions;
    //std::vector<Vector3> normals;
    //std::vector<Vector2> texcoords;
    //std::string line;
    //
    //std::string directory = (_directory + '/');
    //
    //std::ifstream file(directory + _name + ".obj");
    //assert(file.is_open());
    //
    //while (std::getline(file, line)){
    //    std::string identifier;
    //    std::istringstream s(line);
    //    s >> identifier;
    //
    //    if (identifier == "v"){
    //        Vector4 position {};
    //        s >> position.x >> position.y >> position.z;
    //        position.w = 1;
    //        positions.push_back(position);
    //    } else if (identifier == "vt"){
    //        Vector2 texcoord {};
    //        s >> texcoord.x >> texcoord.y;
    //        texcoords.push_back(texcoord);
    //    } else if (identifier == "vn"){
    //        Vector3 normal {};
    //        s >> normal.x >> normal.y >> normal.z;
    //        normals.push_back(normal);
    //    } else if (identifier == "f"){
    //        VertexData triangle[3];
    //        for (auto &faceVertex: triangle) {
    //            std::string vertexDefinition;
    //            s >> vertexDefinition;
    //
    //            std::istringstream v(vertexDefinition);
    //            uint32_t elementIndices[3];
    //
    //            for (unsigned int &elementIndex: elementIndices) {
    //                std::string index;
    //                std::getline(v, index, '/');
    //                elementIndex = std::stoi(index);
    //            }
    //
    //            Vector4 position = positions[elementIndices[0] - 1];
    //            Vector2 texcoord = texcoords[elementIndices[1] - 1];
    //            Vector3 normal = normals[elementIndices[2] - 1];
    //
    //            position.x *= -1;
    //            texcoord.y = 1 - texcoord.y;
    //            normal.x *= -1;
    //
    //            faceVertex = {position, texcoord, normal};
    //        }
    //        modelData.vertices.push_back(triangle[2]);
    //        modelData.vertices.push_back(triangle[1]);
    //        modelData.vertices.push_back(triangle[0]);
    //    } else if (identifier == "mtllib"){
    //        std::string materialFileName;
    //        s >> materialFileName;
    //
    //        modelData.material = LoadMaterialTemplateFile(directory, materialFileName);
    //    }
    //}
    //
    //modelData_ = modelData;

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
