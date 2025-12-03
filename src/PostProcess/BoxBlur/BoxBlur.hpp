#ifndef BoxBlur_HPP_
#define BoxBlur_HPP_
#include "src/PostProcess/IPostEffect.hpp"

class BoxBlur : public IPostEffect{
    struct Material {
        Vector4 color;
    };

    std::unique_ptr<DX12Resource> mr_;
    Material* material_ = nullptr;

public:
    void Initialize() override;
    void Debug() override;

protected:
    void Modifier() override;

public:
    void LoadPreset(const std::string& _presetName) override;
    void SavePreset(const std::string& _presetName) override;
    nlohmann::json SaveParameters() const override;
    void UpdateAnimation(float _t) override;
}; // class BoxBlur

#endif // BoxBlur_HPP_
