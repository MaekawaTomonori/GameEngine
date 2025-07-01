#include "Mesh.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>

#include "Utils.hpp"
#include "Math/Vector3.hpp"

void Mesh::Initialize(DirectXAdapter* _adapter, const std::string &_directory, const std::string &_name) {
    adapter_ = _adapter;

    LoadFile(_directory, _name);
}

void Mesh::Update() {
}

void Mesh::Draw() {
}

void Mesh::LoadFile(const std::string &_directory, const std::string &_name) {
    std::filesystem::path directory(_directory);
    if (exists(directory) || !is_directory(directory)) {
        Utils::Alert("Mesh::LoadFile: Directory does not exist or is not a directory: " + _directory);
        return;
    }

    std::filesystem::path file = directory / _name / _name;

    //obj
    if (exists(file / ".obj")){
        LoadObj(Utils::Convert(directory), _name);
        return;
    }
    //gltf
    if (exists(file / ".gltf")) {
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

    std::string directory = (_directory + _name + '/');

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
