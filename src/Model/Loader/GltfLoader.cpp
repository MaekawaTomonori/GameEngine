#include "GltfLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/anim.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include "Log.hpp"
#include "Utils.hpp"
#include "Math/MathUtils.hpp"

void GltfLoader::LoadModel(const std::string& _name, ResourceRepository* _repository) {
    LoadGltf(ASSETS_FOLDER, _name, _repository);
}

void GltfLoader::LoadGltf(const std::string& _directory, const std::string& _name, ResourceRepository* _repository) {
    Assimp::Importer importer;
    std::string path = _directory + _name + "/" + _name + ".gltf";
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
    if (!scene->HasMeshes()){
        Utils::Alert("GltfLoader::LoadGltf: No meshes found in file: " + path);
        return;
    }
    Log::Send(Log::Level::INFO, "[GLTF Loader] Loaded " + path + " by Assimp");

    std::unique_ptr<ModelData> data_ = std::make_unique<ModelData>();
    data_->name = _name;
    data_->mesh = _name;
    data_->root = LoadNode(scene->mRootNode);

    MeshData meshData;
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0)) {
            Utils::Alert("GltfLoader::LoadGltf: Mesh does not have normals or texture coordinates in file: " + path);
            continue;
        }

        meshData.vertices.reserve(mesh->mNumVertices);
        meshData.indices.reserve(UINT(mesh->mNumFaces) * 3);

        meshData.vertices.resize(mesh->mNumVertices);
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
            const aiVector3D& position = mesh->mVertices[vertexIndex];
            const aiVector3D& normal = mesh->mNormals[vertexIndex];
            const aiVector3D uv = mesh->mTextureCoords[0][vertexIndex];

            Vertex vertexData{};
            vertexData.position = Vector4(-position.x, position.y, position.z, 1.0f);
            vertexData.normal = Vector3(-normal.x, normal.y, normal.z);
            vertexData.uv = Vector2(uv.x, uv.y);
            meshData.vertices[vertexIndex] = vertexData;
        }

        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            const aiFace& face = mesh->mFaces[faceIndex];

            if (face.mNumIndices != 3) {
                Utils::Alert("GltfLoader::LoadGltf: Non-triangular face found in mesh: " + std::to_string(meshIndex) + " in file: " + path);
                return;
            }

            for (uint32_t elementIndex = 0; elementIndex < face.mNumIndices; ++elementIndex) {
                meshData.indices.push_back(face.mIndices[elementIndex]);
            }
        }

        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            aiBone* bone = mesh->mBones[boneIndex];
            if (!bone)continue;

            std::string name = bone->mName.C_Str();

            auto& skinCluster = data_->skinCluster;

            if (!skinCluster.contains(name)){
                JointWeightData& jointWeightData = skinCluster[name];

                aiMatrix4x4 bindPose = bone->mOffsetMatrix.Inverse();
                aiVector3D scale, translate;
                aiQuaternion rotate;
                bindPose.Decompose(scale, rotate, translate);

                jointWeightData.inverseBindPose = MathUtils::Matrix::MakeAffineMatrix({scale.x, scale.y, scale.z}, Quaternion{rotate.x, -rotate.y, -rotate.z, rotate.w}, {-translate.x, translate.y, translate.z}).Inverse();

                jointWeightData.weights.reserve(bone->mNumWeights);

                for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
                    const aiVertexWeight& weight = bone->mWeights[weightIndex];

                    if (weight.mVertexId >= mesh->mNumVertices) {
                        Log::Send(Log::Level::ERR, "GltfLoader::LoadGltf: Vertex index out of bounds in mesh: " + std::to_string(meshIndex) + " in file: " + path);
                        Utils::Alert("Vertex index out of bounds in skin cluster creation");
                        continue;
                    }

                    jointWeightData.weights.push_back({
                        weight.mWeight,
                        weight.mVertexId
                    });
                }
            }
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        if (material && 0 < material->GetTextureCount(aiTextureType_DIFFUSE)){
            aiString texturePath;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS){
                meshData.texture = _directory + _name + "/" + texturePath.C_Str();
            }
        }
    }

    _repository->GetMeshRepository()->Add(_name, meshData);

    data_->skeleton = CreateSkeleton(data_->root);
    data_->animation = LoadAnimation(_directory, _name);

    _repository->GetModelRepository()->Add(_name, std::move(data_));
}

Node GltfLoader::LoadNode(const aiNode* _node) {
    Node result;
    aiVector3D translate, scale;
    aiQuaternion rotate;
    _node->mTransformation.Decompose(scale, rotate, translate);

    Transform transform;
    transform.scale = { scale.x, scale.y, scale.z };
    transform.rotate = Quaternion(rotate.x, -rotate.y, -rotate.z, rotate.w);
    transform.translate = Vector3(-translate.x, translate.y, translate.z);
    result.transform = transform;
    result.local = MathUtils::Matrix::MakeAffineMatrix(transform);

    result.name = _node->mName.C_Str();
    result.children.resize(_node->mNumChildren);
    for (uint32_t i = 0; i < _node->mNumChildren; ++i){
        result.children[i] = LoadNode(_node->mChildren[i]);
    }

    return result;
}

Animation GltfLoader::LoadAnimation(const std::string& _directory, const std::string& _name) {
    Assimp::Importer importer;
    std::string path = _directory + _name + "/" + _name + ".gltf";
    const aiScene* scene = importer.ReadFile(path.c_str(), 0);

    aiAnimation* animation = scene->mAnimations[0];
    Animation animation_;
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
    return animation_;
}

Skeleton GltfLoader::CreateSkeleton(const Node& _root) {
    Skeleton skeleton;
    skeleton.root = CreateJoint(_root, {}, skeleton.joints);

    for (const Joint& joint : skeleton.joints) {
        skeleton.map.emplace(joint.name, joint.index);
    }

    return skeleton;
}

int32_t GltfLoader::CreateJoint(const Node& _node, const std::optional<int32_t>& _parent, std::vector<Joint>& _joints) {
    Joint joint;
    joint.transform = _node.transform;
    joint.local = _node.local;
    joint.space = MathUtils::Matrix::MakeIdentity();
    joint.name = _node.name;
    joint.index = static_cast<int32_t>(_joints.size());
    joint.parent = _parent;
    _joints.push_back(joint);

    for (const Node& child : _node.children) {
        int32_t childIndex = CreateJoint(child, joint.index, _joints);
        _joints[joint.index].children.push_back(childIndex);
    }

    return joint.index;
}
