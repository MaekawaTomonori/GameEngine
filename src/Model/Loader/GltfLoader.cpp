#include "GltfLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/anim.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include "Utils.hpp"
#include "Math/MathUtils.hpp"

void GltfLoader::LoadModel(const std::string& _name, ResourceRepository* _repository) {
    LoadGltf(ASSETS_FOLDER, _name, _repository);
}

void GltfLoader::LoadGltf(const std::string& _directory, const std::string& _name, ResourceRepository* _repository) {
    Assimp::Importer importer;
    std::string path = _directory + _name + "/" + _name + ".gltf";
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    if (!scene->HasMeshes()){
        Utils::Alert("GltfLoader::LoadGltf: No meshes found in file: " + path);
        return;
    }

    std::unique_ptr<ModelData> data_ = std::make_unique<ModelData>();
    data_->name = _name;
    data_->mesh = _name;

    MeshData meshData;
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0)) {
            Utils::Alert("GltfLoader::LoadGltf: Mesh does not have normals or texture coordinates in file: " + path);
            continue;
        }

        meshData.vertices.resize(mesh->mNumVertices);
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
            aiVector3D& position = mesh->mVertices[vertexIndex];
            aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D texcoord = mesh->mTextureCoords[0][vertexIndex];

            Vertex vertexData{};
            vertexData.position = Vector4(-position.x, position.y, position.z, 1.0f);
            vertexData.normal = Vector3(-normal.x, normal.y, normal.z);
            vertexData.uv = Vector2(texcoord.x, texcoord.y);
            meshData.vertices[vertexIndex] = vertexData;
        }

        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];

            if (face.mNumIndices != 3) {
                Utils::Alert("GltfLoader::LoadGltf: Non-triangular face found in mesh: " + std::to_string(meshIndex) + " in file: " + path);
                return;
            }

            for (uint32_t element = 0; element < face.mNumIndices; ++element){
                meshData.indices.push_back(face.mIndices[element]);
            }

            for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                if (material->GetTextureCount(aiTextureType_DIFFUSE)){
                    aiString texturePath;
                    material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
                    meshData.texture = _directory + _name + "/" + texturePath.C_Str();
                }
            }
        }
    }

    _repository->GetMeshRepository()->Add(_name, meshData);

    data_->root = LoadNode(scene->mRootNode);
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
    result.children.reserve(_node->mNumChildren);
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

void GltfLoader::UpdateSkeleton(Skeleton& _skeleton) {
    for (Joint& joint : _skeleton.joints){
        joint.local = MathUtils::Matrix::MakeAffineMatrix(joint.transform);
        if (joint.parent){
            joint.space = joint.local * _skeleton.joints[*joint.parent].space;
        } else{
            joint.space = joint.local;
        }
    }
}

void GltfLoader::ApplyAnimation(float _time, ModelData _data) {
    for (Joint& joint : _data.skeleton.joints) {
        if (_data.animation.nodeAnimations.contains(joint.name)) {
            const NodeAnimation& rna = _data.animation.nodeAnimations[joint.name];
            (void)rna;
            (void) _time;
            //joint.transform.scale = rna.scale.keyframes;
        }
    }
}
