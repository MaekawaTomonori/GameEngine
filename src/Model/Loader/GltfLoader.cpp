#include "GltfLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Utils.hpp"
#include "Math/MathUtils.hpp"

Model* GltfLoader::LoadModel(const std::string& _directory, const std::string& _name, ResourceRepository* _repository) {

}

void GltfLoader::LoadGltf(const std::string& _directory, const std::string& _name) {
    Assimp::Importer importer;
    std::string path = _directory + "/" + _name + ".gltf";
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    if (!scene->HasMeshes()){
        Utils::Alert("GltfLoader::LoadGltf: No meshes found in file: " + path);
        return;
    }

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0)) {
            Utils::Alert("GltfLoader::LoadGltf: Mesh does not have normals or texture coordinates in file: " + path);
            continue;
        }

        data_.vertices.resize(mesh->mNumVertices);
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
            aiVector3D& position = mesh->mVertices[vertexIndex];
            aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D texcoord = mesh->mTextureCoords[0][vertexIndex];

            VertexData vertexData{};
            vertexData.position = Vector4(-position.x, position.y, position.z, 1.0f);
            vertexData.normal = Vector3(-normal.x, normal.y, normal.z);
            vertexData.texcoord = Vector2(texcoord.x, texcoord.y);
            data_.vertices[vertexIndex] = vertexData;
        }

        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            aiFace& face = mesh->mFaces[faceIndex];

            if (face.mNumIndices == 3) {
                Utils::Alert("GltfLoader::LoadGltf: Mesh has non-triangular faces in file: " + path);
            }

            for (uint32_t element = 0; element < face.mNumIndices; ++element){
                data_.indices.push_back(face.mIndices[element]);
            }
        }
    }

    data_.root = LoadNode(scene->mRootNode);
}

Node GltfLoader::LoadNode(const aiNode* _node) {
    Node result;
    aiVector3D translate, scale;
    aiQuaternion rotate;
    Transform transform;
    _node->mTransformation.Decompose(scale, rotate, translate);
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

void GltfLoader::LoadAnimation(const std::string& _directory, const std::string& _name) {
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

GltfLoader::MaterialData GltfLoader::LoadMaterialTemplateFile(std::string &_directory, std::string &_name) {
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

void GltfLoader::UpdateSkeleton() {
    for (Joint& joint : skeleton_.joints){
        joint.local = MathUtils::Matrix::MakeAffineMatrix(joint.transform);
        if (joint.parent){
            joint.space = joint.local * skeleton_.joints[*joint.parent].space;
        } else{
            joint.space = joint.local;
        }
    }
}

void GltfLoader::ApplyAnimation(float _time) {
    for (Joint& joint : skeleton_.joints) {
        if (animation_.nodeAnimations.contains(joint.name)) {
            const NodeAnimation& rna = animation_.nodeAnimations[joint.name];
            joint.transform.scale = rna.scale.keyframes;
        }
    }
}
