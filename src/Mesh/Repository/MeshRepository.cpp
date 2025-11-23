#include "MeshRepository.hpp"

#include "Log.hpp"
#include "Model.hpp"
#include "Utils.hpp"

void MeshRepository::Initialize(DirectXAdapter *_adapter) {
    adapter_ = _adapter;
}

void MeshRepository::Add(const std::string& _name, const MeshData& _raw) {
    if (data_.contains(_name))return;
    data_[_name] = std::move(_raw);
}

MeshData MeshRepository::Get(const std::string& _name) {
    if (data_.contains(_name)){return data_[_name];}
    Model::Load(_name);
    if (data_.contains(_name)){return data_[_name];}

    Utils::Alert("MeshRepository: Mesh not found: " + _name);
    
    return {};
}
