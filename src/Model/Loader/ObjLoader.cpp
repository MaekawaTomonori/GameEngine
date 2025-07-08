#include "ObjLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Utils.hpp"
#include "src/Mesh/Mesh.hpp"

ModelData* ObjLoader::LoadModel(const std::string& _directory, const std::string& _name, ResourceRepository* _repository) {
    if (ModelData* data = _repository->GetModelRepository()->Get(_name)) {
        return data;
    }


    std::unique_ptr<ModelData> data = std::make_unique<ModelData>();
    data->name = _name;
    data->mesh = _name + ".obj";

    Assimp::Importer importer;
    std::string path = _directory + "/" + _name + ".obj";
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    if (!scene->HasMeshes()){
        Utils::Alert("Mesh::LoadObj: No meshes found in file: " + path);
        return nullptr;
    }
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex){
        aiMesh* mesh = scene->mMeshes[meshIndex];
        RawMesh raw;

        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex){
            aiFace& face = mesh->mFaces[faceIndex];

            for (uint32_t element = 0; element < face.mNumIndices; ++element){
                uint32_t vertex = face.mIndices[element];
                aiVector3D& position = mesh->mVertices[vertex];
                aiVector3D& normal = mesh->mNormals[vertex];
                aiVector3D uv = mesh->mTextureCoords[0][vertex];

                RawVertex vertexData;
                vertexData.position = Vector4(position.x, position.y, position.z, 1.0f);
                vertexData.uv = Vector2(uv.x, uv.y);
                vertexData.normal = Vector3(normal.x, normal.y, normal.z);
                
                vertexData.position.x *= -1; // Flip X axis
                vertexData.normal.x *= -1; // Flip X axis

                raw.vertices.push_back(vertexData);
            }

            for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex){
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                if (material->GetTextureCount(aiTextureType_DIFFUSE)){
                    aiString texturePath;
                    material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
                    raw.texture = _directory + "/" + texturePath.C_Str();
                }
            }
        }
        _repository.
    }
    _repository->GetModelRepository()->Add(_name, std::move(data));

    return _repository->GetModelRepository()->Get(_name);
}
