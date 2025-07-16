#include "MeshRepository.hpp"

void MeshRepository::Initialize(DirectXAdapter *_adapter) {
    adapter_ = _adapter;
}

void MeshRepository::Add(const std::string& _name, MeshData _raw) {
    if (data_.contains(_name))return;
    data_[_name] = std::move(_raw);
}

MeshData MeshRepository::Get(const std::string& _name) {
    auto it = data_.find(_name);
    if (it != data_.end()){
        return it->second;
    }
    return {}; // Return empty MeshData if not found
}
