#ifndef ModelLoader_HPP_
#define ModelLoader_HPP_
#include "IModelLoader.hpp"

class ObjLoader : public IModelLoader{
public:
    ModelData* LoadModel(const std::string& _directory, const std::string& _name, ResourceRepository* _repository) override;
}; // class ObjLoader

#endif // ModelLoader_HPP_
