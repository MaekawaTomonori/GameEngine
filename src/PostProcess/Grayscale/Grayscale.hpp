#ifndef Grayscale_HPP_
#define Grayscale_HPP_
#include "src/PostProcess/IPostEffect.hpp"

class Grayscale : public IPostEffect{
    struct Material {
        Vector4 color;
    };

    const Vector4 GRAY = { 0.2125f, 0.7154f, 0.0721f, 1.0f };
    const Vector4 SEPIA = { 1.f, 74.f / 107.f, 43.f / 107.f, 1.0f };

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
}; // class Grayscale

#endif // Grayscale_HPP_
