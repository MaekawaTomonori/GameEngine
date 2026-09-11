#include "MeshRepository.hpp"

#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Model/Loader/ModelLoaderFactory.hpp"

void MeshRepository::Initialize(DirectXAdapter *_adapter) {
    adapter_ = _adapter;
}

void MeshRepository::Add(const std::string& _name, const MeshData& _raw) {
    if (data_.contains(_name))return;
    data_[_name] = std::move(_raw);
}

const MeshData& MeshRepository::Get(const std::string& _name) {
    if (data_.contains(_name)){return data_.at(_name);}
    ModelLoaderFactory::Load(_name, Singleton<ModelCommon>::GetInstance()->GetResourceRepository());
    if (data_.contains(_name)){return data_.at(_name);}

    Utils::Alert("MeshRepository: Mesh not found: " + _name);

    static const MeshData EMPTY{};
    return EMPTY;
}

bool MeshRepository::Contains(const std::string& _name) const {
    return data_.contains(_name);
}
