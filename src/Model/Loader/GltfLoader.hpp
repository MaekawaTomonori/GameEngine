#ifndef GltfLoader_HPP_
#define GltfLoader_HPP_
#include <string>

#include "IModelLoader.hpp"
#include "src/Model/Node/Node.hpp"

class GltfLoader : public IModelLoader{
public:
    Model* LoadModel(const std::string& _directory, const std::string& _name, ResourceRepository* _repository) override;
    
private:
    void LoadGltf(const std::string& _directory, const std::string& _name);

    static Node LoadNode(const aiNode* _node);

    void LoadAnimation(const std::string& _directory, const std::string& _name);

    Skeleton CreateSkeleton(const Node& _root);

    int32_t CreateJoint(const Node& _node, const std::optional<int32_t>& _parent, std::vector<Joint>& _joints);

    void UpdateSkeleton();
    void ApplyAnimation(float _time);
}; // class GltfLoader

#endif // GltfLoader_HPP_
