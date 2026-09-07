#include "Factory/PostEffectFactory.hpp"

#include <ranges>

#include "Log.hpp"
#include "src/PostProcess/IPostEffect.hpp"

void PostEffectFactory::Register(const std::string& _type, const CreateFunc& _creator) {
    creators_[_type] = _creator;
    Log::Send(Log::Level::INFO, "PostEffectRegistered : " + _type);
}

std::unique_ptr<IPostEffect> PostEffectFactory::Create(const std::string& _type) {
    if (creators_.contains(_type)) {
        return creators_[_type]();
    }

    Log::Send(Log::Level::ERR, "PostEffect type not found: " + _type);
    return nullptr;
}

std::vector<std::string> PostEffectFactory::GetRegisteredTypes() const {
    std::vector<std::string> types;
    types.reserve(creators_.size());
    for (const auto& type : creators_ | std::views::keys) {
        types.push_back(type);
    }
    return types;
}
