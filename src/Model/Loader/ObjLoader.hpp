#ifndef ModelLoader_HPP_
#define ModelLoader_HPP_
#include "IModelLoader.hpp"

class ObjLoader : public IModelLoader{
public:
    void LoadModel(const std::string& _name, GESTD::ReferencePtr<ResourceRepository> _repository) override;
}; // class ObjLoader

#endif // ModelLoader_HPP_
