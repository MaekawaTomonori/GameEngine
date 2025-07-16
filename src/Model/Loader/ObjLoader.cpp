#include "ObjLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Utils.hpp"

void ObjLoader::LoadModel(const std::string& _name, ResourceRepository* _repository) {
    std::unique_ptr<ModelData> data = std::make_unique<ModelData>();
    data->name = _name;
    data->mesh = _name;

    Assimp::Importer importer;
    std::string path = ASSETS_FOLDER + _name + "/" + _name + ".obj";
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    if (!scene->HasMeshes()){
        Utils::Alert("Mesh::LoadObj: No meshes found in file: " + path);
    }
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex){
        aiMesh* mesh = scene->mMeshes[meshIndex];
        MeshData raw;

        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex){
            aiFace& face = mesh->mFaces[faceIndex];

            for (uint32_t element = 0; element < face.mNumIndices; ++element){
                uint32_t vertex = face.mIndices[element];
                aiVector3D& position = mesh->mVertices[vertex];
                aiVector3D& normal = mesh->mNormals[vertex];
                aiVector3D uv = mesh->mTextureCoords[0][vertex];

                Vertex vertexData;
                vertexData.position = Vector4(position.x, position.y, position.z, 1.0f);
                vertexData.uv = Vector2(uv.x, uv.y);
                vertexData.normal = Vector3(normal.x, normal.y, normal.z);
                
                vertexData.position.x *= -1; // Flip X axis
                vertexData.normal.x *= -1; // Flip X axis

                raw.vertices.push_back(vertexData);
            }
        }

        for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex){
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            if (material->GetTextureCount(aiTextureType_DIFFUSE)){
                aiString texturePath;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
                raw.texture = ASSETS_FOLDER + _name + "/" + texturePath.C_Str();
            }
        }

        _repository->GetMeshRepository()->Add(_name, raw);
    }
    _repository->GetModelRepository()->Add(_name, std::move(data));
}
