#ifndef Vignette_HPP_
#define Vignette_HPP_
#include "src/PostProcess/IPostEffect.hpp"
#include <unordered_map>
#include <vector>
#include <string>

class Vignette : public IPostEffect{
    struct Material {
        Vector4 color;
        float intensity;
        float scale;
        float pad[2];
    };

    struct KeyframeData {
        float intensity;
        float scale;
        Vector4 color;
    };

    std::unique_ptr<DX12Resource> mr_;
    Material* material_ = nullptr;

    // キーフレームデータ
    std::unordered_map<std::string, KeyframeData> keyframes_;
    std::vector<std::string> keyframeOrder_;

public:
    void Initialize() override;
    void Debug() override;

    // プリセットシステム
    void LoadPreset(const std::string& _presetName) override;
    void SavePreset(const std::string& _presetName) override;
    nlohmann::json SaveParameters() const override;
    void UpdateAnimation(float _t) override;

protected:
    void Modifier() override;
}; // class Vignette

#endif // Vignette_HPP_
