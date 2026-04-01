#ifndef GltfLoader_HPP_
#define GltfLoader_HPP_
#include <string>
#include <assimp/scene.h>

#include "IModelLoader.hpp"
#include "src/Model/Node/Node.hpp"

class GltfLoader : public IModelLoader{
public:
    void LoadModel(const std::string& _name, GESTD::WeakPtr<ResourceRepository> _repository) override;

private:
    void LoadGltf(const std::string& _directory, const std::string& _name, const GESTD::WeakPtr<ResourceRepository> _repository) const;

    static Node LoadNode(const aiNode* _node);

    static std::optional<Animation> LoadAnimation(const aiScene* _scene, const std::string& _name);

    static Skeleton CreateSkeleton(const Node& _root);

    static int32_t CreateJoint(const Node& _node, const std::optional<int32_t>& _parent, std::vector<Joint>& _joints);

    MeshData LoadMesh(const aiScene* _scene, const std::string& _name, ModelData& _model) const;

    static void LoadVertexData(const aiMesh* _mesh, MeshData& _data);
    static void LoadIndexData(const aiMesh* _mesh, MeshData& _data);
    static void LoadBones(const aiMesh* _mesh, ModelData& _model);
}; // class GltfLoader

#endif // GltfLoader_HPP_
