#include "Mesh.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>

#include "DebugUI.hpp"
#include "Singleton.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Math/Vector3.hpp"
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
    memcpy(vd_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

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

    commandList_->DrawInstanced(static_cast<UINT>(modelData_.vertices.size()), 1, 0, 0);
}

void Mesh::Debug() {
	
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
        //LoadGltf((file/".gltf").c_str());
        return;
    }

    Utils::Alert("Mesh::LoadFile: No valid mesh file found in directory: " + _directory);
}

void Mesh::LoadObj(const std::string& _directory, const std::string &_name) {
    ModelData modelData {};
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::string line;

    std::string directory = (_directory + '/');

    std::ifstream file(directory + _name + ".obj");
    assert(file.is_open());

    while (std::getline(file, line)){
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "v"){
            Vector4 position {};
            s >> position.x >> position.y >> position.z;
            position.w = 1;
            positions.push_back(position);
        } else if (identifier == "vt"){
            Vector2 texcoord {};
            s >> texcoord.x >> texcoord.y;
            texcoords.push_back(texcoord);
        } else if (identifier == "vn"){
            Vector3 normal {};
            s >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (identifier == "f"){
            VertexData triangle[3];
            for (auto &faceVertex: triangle) {
                std::string vertexDefinition;
                s >> vertexDefinition;

                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];

                for (unsigned int &elementIndex: elementIndices) {
                    std::string index;
                    std::getline(v, index, '/');
                    elementIndex = std::stoi(index);
                }

                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];

                position.x *= -1;
                texcoord.y = 1 - texcoord.y;
                normal.x *= -1;

                faceVertex = {position, texcoord, normal};
            }
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        } else if (identifier == "mtllib"){
            std::string materialFileName;
            s >> materialFileName;

            modelData.material = LoadMaterialTemplateFile(directory, materialFileName);
        }
    }

    modelData_ = modelData;
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
