#ifndef UiKeyboardModule_HPP_
#define UiKeyboardModule_HPP_

#include <cstdint>
#include <vector>
#include "Ui/Module/IModule.hpp"

namespace Ui {

    class KeyboardModule final : public IModule {
    public:
        struct KeyBindings {
            std::vector<uint8_t> up      = { 0xC8u }; // DIK_UP
            std::vector<uint8_t> down    = { 0xD0u }; // DIK_DOWN
            std::vector<uint8_t> left    = { 0xCBu }; // DIK_LEFT
            std::vector<uint8_t> right   = { 0xCDu }; // DIK_RIGHT
            std::vector<uint8_t> execute = { 0x1Cu }; // DIK_RETURN
        };

    private:
        KeyBindings keys_;

    public:
        std::string_view GetType() const override { return "Keyboard"; }
        void Update(Canvas& canvas, const Input& input) override;
        void LoadFromJson(const nlohmann::json& j) override;
        void SaveToJson(nlohmann::json& j) const override;

        void SetKeyBindings(const KeyBindings& keys) { keys_ = keys; }
        const KeyBindings& GetKeyBindings() const { return keys_; }
    };

} // namespace Ui

#endif // UiKeyboardModule_HPP_
