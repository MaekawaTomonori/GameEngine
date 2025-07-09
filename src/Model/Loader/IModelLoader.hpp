#ifndef IModelLoader_HPP_
#define IModelLoader_HPP_
#include "Model.hpp"
#include "src/ResourceRepository/ResourceRepository.hpp"

class IModelLoader {
protected:
    const std::string ASSETS_FOLDER = "Assets/Resources/";

public:
    virtual ~IModelLoader() = default;
    virtual void LoadModel(const std::string& _name, ResourceRepository* _repository) = 0;
}; // class IModelLoader

#endif // IModelLoader_HPP_
