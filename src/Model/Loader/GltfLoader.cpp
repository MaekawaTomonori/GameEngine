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

void GltfLoader::LoadGltf(const std::string& _directory, const std::string& _name, const ResourceRepository* _repository) {
    Assimp::Importer importer;
    std::string path = _directory + _name + "/" + _name + ".gltf";
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
    if (!scene) {
        Utils::Alert("GltfLoader::LoadGltf: Failed to load file: " + path + "\nError: " + importer.GetErrorString());
        Log::Send(Log::Level::ERR, "[GLTF Loader] Failed to load " + path + ": " + importer.GetErrorString());
        return;
    }
    if (!scene->HasMeshes()){
        Utils::Alert("GltfLoader::LoadGltf: No meshes found in file: " + path);
        return;
    }
    Log::Send(Log::Level::INFO, "[GLTF Loader] Loaded " + path + " by Assimp");

    // Model Loader
    std::unique_ptr<ModelData> model_ = std::make_unique<ModelData>();
    model_->name = _name;
    model_->mesh = _name;
    model_->root = LoadNode(scene->mRootNode);
    model_->animation = LoadAnimation(scene, _name);
    model_->skeleton = CreateSkeleton(model_->root);

    //Mesh
    MeshData mesh_ = LoadMesh(scene, _name, *model_);

    _repository->GetMeshRepository()->Add(_name, mesh_);
    _repository->GetModelRepository()->Add(_name, std::move(model_));
}

Node GltfLoader::LoadNode(const aiNode* _node) {
    Node node;
    node.name = _node->mName.C_Str();

    aiVector3D scale, translate;
    aiQuaternion rotate;
    _node->mTransformation.Decompose(scale, rotate, translate);
    node.transform.scale = Vector3{ .x= scale.x, .y= scale.y, .z= scale.z };
    node.transform.rotate = Quaternion{.x= rotate.x, .y= -rotate.y, .z= -rotate.z, .w= rotate.w};
    node.transform.translate = Vector3{ .x = -translate.x, .y = translate.y, .z = translate.z };
    node.local = MathUtils::Matrix::MakeAffineMatrix(node.transform);

    for (unsigned int i = 0; i < _node->mNumChildren; ++i){
        node.children.push_back(LoadNode(_node->mChildren[i]));
    }

    return node;
}

std::optional<Animation> GltfLoader::LoadAnimation(const aiScene* _scene, const std::string& _name) {
    Animation result;

    if (_scene->mNumAnimations <= 0) {
        Log::Send(Log::Level::WARNING, "GltfLoader::LoadAnimation: No animations found in file: " + _name);
        return std::nullopt;
    }

    aiAnimation* animation = _scene->mAnimations[0];
    float tps = (animation->mTicksPerSecond == 0.) ? 1.f : static_cast<float>(animation->mTicksPerSecond);

    result.duration = static_cast<float>(animation->mDuration / tps);

    for (uint32_t channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
        aiNodeAnim* channel = animation->mChannels[channelIndex];
        NodeAnimation& nodeAnimation = result.nodeAnimations[channel->mNodeName.C_Str()];

        uint32_t key;
        for (key = 0; key < channel->mNumPositionKeys; ++key) {
            aiVectorKey& position = channel->mPositionKeys[key];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float>(position.mTime / tps);
            keyframe.value = {-position.mValue.x, position.mValue.y, position.mValue.z};
            nodeAnimation.translate.keyframes.push_back(keyframe);
        }

        for (key = 0; key < channel->mNumRotationKeys; ++key) {
            aiQuatKey& rotation = channel->mRotationKeys[key];
            KeyframeQuaternion keyframe;
            keyframe.time = static_cast<float>(rotation.mTime / tps);
            keyframe.value = { rotation.mValue.x, -rotation.mValue.y, -rotation.mValue.z, rotation.mValue.w };
            nodeAnimation.rotation.keyframes.push_back(keyframe);
        }

        for (key = 0; key < channel->mNumScalingKeys; ++key) {
            aiVectorKey& scale = channel->mScalingKeys[key];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float>(scale.mTime / tps);
            keyframe.value = { scale.mValue.x, scale.mValue.y, scale.mValue.z };
            nodeAnimation.scale.keyframes.push_back(keyframe);
        }
    }

    return result;
}

Skeleton GltfLoader::CreateSkeleton(const Node& _root) {
    Skeleton skeleton;
    skeleton.root = CreateJoint(_root, std::nullopt, skeleton.joints);

    for (const Joint& joint : skeleton.joints) {
        skeleton.map.emplace(joint.name, joint.index);
    }

    return skeleton;
}

int32_t GltfLoader::CreateJoint(const Node& _node, const std::optional<int32_t>& _parent, std::vector<Joint>& _joints) {
    Joint joint;
    joint.name = _node.name;
    joint.local = _node.local;
    joint.space = MathUtils::Matrix::MakeIdentity();
    joint.transform = _node.transform;
    joint.index = static_cast<int32_t>(_joints.size());
    joint.parent = _parent;
    _joints.push_back(joint);

    for (const Node& child : _node.children) {
        int32_t childIndex = CreateJoint(child, joint.index, _joints);
        _joints[joint.index].children.push_back(childIndex);
    }

    return joint.index;
}

MeshData GltfLoader::LoadMesh(const aiScene* _scene, const std::string& _name, ModelData& _model) const {
    MeshData data;
    for (uint32_t meshIndex = 0; meshIndex < _scene->mNumMeshes; ++meshIndex){
        aiMesh* mesh = _scene->mMeshes[meshIndex];
        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0)) {
            Log::Send(Log::Level::ERR, "GltfLoader::LoadMesh: Mesh does not have normals or texture coordinates");
            Utils::Alert("Mesh does not have normals or texture coordinates");
            continue;
        }

        LoadVertexData(mesh, data);

        LoadIndexData(mesh, data);
        
        LoadBones(mesh, _model);

        for (uint32_t materialIndex = 0; materialIndex < _scene->mNumMaterials; ++materialIndex){
            aiMaterial* material = _scene->mMaterials[materialIndex];
            if (material->GetTextureCount(aiTextureType_DIFFUSE)){
                aiString texturePath;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
                data.texture = ASSETS_FOLDER + _name + "/" + texturePath.C_Str();
            }
        }
    }
    return data;
}

void GltfLoader::LoadVertexData(const aiMesh* _mesh, MeshData& _data) {
    uint32_t vertexCount = _mesh->mNumVertices;
    _data.vertices.resize(vertexCount);
    for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex){
        aiVector3D& position = _mesh->mVertices[vertexIndex];
        aiVector3D& normal = _mesh->mNormals[vertexIndex];
        aiVector3D uv = _mesh->mTextureCoords[0][vertexIndex];

        _data.vertices[vertexIndex].position = Vector4{ -position.x, position.y, position.z, 1.0f };
        _data.vertices[vertexIndex].normal = Vector3{ -normal.x, normal.y, normal.z };
        _data.vertices[vertexIndex].uv = Vector2{ uv.x, uv.y };
    }
}

void GltfLoader::LoadIndexData(const aiMesh* _mesh, MeshData& _data) {
    for (uint32_t faceIndex = 0; faceIndex < _mesh->mNumFaces; ++faceIndex){
        aiFace& face = _mesh->mFaces[faceIndex];

        if (face.mNumIndices != 3){
            Log::Send(Log::Level::ERR, "GltfLoader::LoadMesh: Face does not have 3 indices");
            Utils::Alert("Face does not have 3 indices");
            continue;
        }

        for (uint32_t element = 0; element < face.mNumIndices; ++element){
            _data.indices.push_back(face.mIndices[element]);
        }
    }
}

void GltfLoader::LoadBones(const aiMesh* _mesh, ModelData& _model) {
    for (uint32_t boneIndex = 0; boneIndex < _mesh->mNumBones; ++boneIndex){
        aiBone* bone = _mesh->mBones[boneIndex];
        std::string jointName = bone->mName.C_Str();
        JointWeightData& jointWeight = _model.skinCluster[jointName];

        // bone->mOffsetMatrix is already the inverse bind pose matrix
        aiMatrix4x4 inverseBindPose = bone->mOffsetMatrix;
        aiVector3D scale, translate;
        aiQuaternion rotate;

        inverseBindPose.Decompose(scale, rotate, translate);
        jointWeight.inverseBindPose = MathUtils::Matrix::MakeAffineMatrix(
            Vector3{ .x = scale.x, .y = scale.y, .z = scale.z },
            Quaternion{ .x = rotate.x, .y = -rotate.y, .z = -rotate.z, .w = rotate.w },
            Vector3{ .x = -translate.x, .y = translate.y, .z = translate.z }
        );

        for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex){
            jointWeight.weights.push_back({
                .weight = bone->mWeights[weightIndex].mWeight,
                .index = bone->mWeights[weightIndex].mVertexId
            });
        }
    }
}
