#include "ModelRepository.hpp"

void ModelRepository::Add(const std::string& _name, std::unique_ptr<ModelData> _model) {
    models_.emplace(_name, std::move(_model));
}

ModelData* ModelRepository::Get(const std::string& _name) {
    auto it = models_.find(_name);
    if (it != models_.end()){
        return it->second.get();
    }
    return nullptr;
}

bool ModelRepository::Contains(const std::string& _name) const {
    return models_.contains(_name);
}
