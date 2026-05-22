#ifndef UiMouseModule_HPP_
#define UiMouseModule_HPP_

#include <array>
#include <memory>
#include <string>

#include "Math/Vector2.hpp"
#include "Ui/Module/IModule.hpp"

class Sprite;

namespace Ui {

    class MouseModule final : public IModule {
        std::string          textureName_    = "white_x16.png";
        std::string          pendingTexture_;
        std::unique_ptr<Sprite> sprite_;
        Vector2              mousePos_ {};
        Vector2              mouseSize_{};
        float                hitPadding_ = 8.f;

        void ApplyPendingTexture();

    public:
        std::string_view GetType() const override { return "Mouse"; }
        void Update(Canvas& canvas, const Input& input) override;
        void Draw() const override;
        void Debug();

        void LoadFromJson(const nlohmann::json& j) override;
        void SaveToJson(nlohmann::json& j) const override;

        void               SetTexture(const std::string& _name);
        const std::string& GetTextureName() const {
            return pendingTexture_.empty() ? textureName_ : pendingTexture_;
        }
    };

} // namespace Ui

#endif // UiMouseModule_HPP_
