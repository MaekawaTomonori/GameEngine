#include "ModelLoaderFactory.hpp"

#include <filesystem>
#include <memory>

#include "GltfLoader.hpp"
#include "IModelLoader.hpp"
#include "Log.hpp"
#include "ObjLoader.hpp"
#include "Utils.hpp"
#include "src/Model/Repository/ModelRepository.hpp"
#include "src/ResourceRepository/ResourceRepository.hpp"

void ModelLoaderFactory::Load(const std::string& _name, const GESTD::ReferencePtr<ResourceRepository>& _repository) {
    if (_repository->GetModelRepository()->Contains(_name)) {
        return;
    }

    std::unique_ptr<IModelLoader> loader;
    if (std::filesystem::exists("Assets/Resources/" + _name + "/" + _name + ".obj")) {
        loader = std::make_unique<ObjLoader>();
    }
    else if (std::filesystem::exists("Assets/Resources/" + _name + "/" + _name + ".gltf")) {
        loader = std::make_unique<GltfLoader>();
    }
    else {
        Log::Send(Log::Level::ERR, "ModelLoaderFactory::Load: Model not found: " + _name);
        Utils::Alert("Model not found: " + _name);
        return;
    }
    loader->LoadModel(_name, _repository);
}
