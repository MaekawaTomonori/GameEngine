#include "Ui/Module/KeyboardModule.hpp"
#include "Ui/UserInterface.hpp"
#include "Input.hpp"

#include <algorithm>
#include <ranges>

namespace Ui {

    namespace {
        uint8_t KeyNameToCode(const std::string& name) {
            for (const auto& entry : KeyCodeMap) {
                if (entry.name == name) return static_cast<uint8_t>(entry.code);
            }
            return 0u;
        }

        const char* KeyCodeToName(uint8_t code) {
            for (const auto& entry : KeyCodeMap) {
                if (entry.code == code) return entry.name;
            }
            return "UNKNOWN";
        }

        bool AnyTrigger(const std::vector<uint8_t>& keys, const Input& input) {
            return std::ranges::any_of(keys, [&](uint8_t k) { return input.IsTrigger(k); });
        }

        std::vector<uint8_t> LoadKeys(const nlohmann::json& arr) {
            std::vector<uint8_t> result;
            if (arr.is_array()) {
                for (const auto& v : arr) {
                    if (v.is_string()) {
                        if (uint8_t code = KeyNameToCode(v.get<std::string>())) {
                            result.push_back(code);
                        }
                    }
                }
            } else if (arr.is_string()) {
                if (uint8_t code = KeyNameToCode(arr.get<std::string>())) {
                    result.push_back(code);
                }
            }
            return result;
        }

        nlohmann::json SaveKeys(const std::vector<uint8_t>& keys) {
            nlohmann::json arr = nlohmann::json::array();
            for (uint8_t code : keys) arr.push_back(KeyCodeToName(code));
            return arr;
        }
    } // namespace

    void KeyboardModule::Update(Canvas& canvas, const Input& input) {
        if (canvas.GetPhase() == Canvas::Phase::Inactive) return;

        if (AnyTrigger(keys_.up,      input)) canvas.MoveCursorDir(Canvas::CursorDir::Up);
        if (AnyTrigger(keys_.down,    input)) canvas.MoveCursorDir(Canvas::CursorDir::Down);
        if (AnyTrigger(keys_.left,    input)) canvas.MoveCursorDir(Canvas::CursorDir::Left);
        if (AnyTrigger(keys_.right,   input)) canvas.MoveCursorDir(Canvas::CursorDir::Right);
        if (AnyTrigger(keys_.execute, input)) canvas.ExecuteCursor();
    }

    void KeyboardModule::LoadFromJson(const nlohmann::json& j) {
        if (!j.contains("Keys")) return;
        const auto& keys = j["Keys"];

        if (keys.contains("Up"))      { auto v = LoadKeys(keys["Up"]);      if (!v.empty()) keys_.up      = v; }
        if (keys.contains("Down"))    { auto v = LoadKeys(keys["Down"]);    if (!v.empty()) keys_.down    = v; }
        if (keys.contains("Left"))    { auto v = LoadKeys(keys["Left"]);    if (!v.empty()) keys_.left    = v; }
        if (keys.contains("Right"))   { auto v = LoadKeys(keys["Right"]);   if (!v.empty()) keys_.right   = v; }
        if (keys.contains("Execute")) { auto v = LoadKeys(keys["Execute"]); if (!v.empty()) keys_.execute = v; }
    }

    void KeyboardModule::SaveToJson(nlohmann::json& j) const {
        j["Keys"] = {
            { "Up",      SaveKeys(keys_.up)      },
            { "Down",    SaveKeys(keys_.down)    },
            { "Left",    SaveKeys(keys_.left)    },
            { "Right",   SaveKeys(keys_.right)   },
            { "Execute", SaveKeys(keys_.execute) },
        };
    }

} // namespace Ui
