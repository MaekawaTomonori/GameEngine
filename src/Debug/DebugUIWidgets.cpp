#include "DebugUIWidgets.hpp"

#include "imgui.h"

namespace {
    void UseFullWidth() {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    }
}

namespace DebugUIWidgets {
    bool DragFloat(const char* _label, float* _v, float _speed, float _min, float _max) {
        ImGui::Text("%s", _label);
        UseFullWidth();
        return ImGui::DragFloat(("##" + std::string(_label)).c_str(), _v, _speed, _min, _max);
    }

    bool DragFloat3(const char* _label, float* _v, float _speed, float _min, float _max) {
        ImGui::Text("%s", _label);
        UseFullWidth();
        return ImGui::DragFloat3(("##" + std::string(_label)).c_str(), _v, _speed, _min, _max);
    }

    bool DragInt(const char* _label, int* _v, float _speed, int _min, int _max) {
        ImGui::Text("%s", _label);
        UseFullWidth();
        return ImGui::DragInt(("##" + std::string(_label)).c_str(), _v, _speed, _min, _max);
    }

    bool ColorEdit4(const char* _label, float* _v) {
        ImGui::Text("%s", _label);
        UseFullWidth();
        return ImGui::ColorEdit4(("##" + std::string(_label)).c_str(), _v);
    }

    bool Checkbox(const char* _label, bool* _v) {
        ImGui::Text("%s", _label);
        return ImGui::Checkbox(("##" + std::string(_label)).c_str(), _v);
    }

    bool Combo(const char* _label, int* _current, const char* const _items[], int _count) {
        ImGui::Text("%s", _label);
        UseFullWidth();
        return ImGui::Combo(("##" + std::string(_label)).c_str(), _current, _items, _count);
    }

    bool KeyCombo(const char* _label, const std::vector<std::string>& _keys, std::string& _selected) {
        ImGui::Text("%s", _label);

        if (_keys.empty()) {
            ImGui::TextDisabled("  (選択可能な項目がありません)");
            return false;
        }

        std::vector<std::string> options;
        options.reserve(_keys.size() + 1);
        options.push_back("(None)");
        options.insert(options.end(), _keys.begin(), _keys.end());

        int currentIndex = 0;
        for (size_t i = 1; i < options.size(); ++i) {
            if (options[i] == _selected) {
                currentIndex = static_cast<int>(i);
                break;
            }
        }

        std::vector<const char*> items;
        items.reserve(options.size());
        for (const auto& option : options) {
            items.push_back(option.c_str());
        }

        UseFullWidth();
        if (ImGui::Combo(("##" + std::string(_label)).c_str(), &currentIndex, items.data(), static_cast<int>(items.size()))) {
            _selected = (currentIndex == 0) ? "" : options[currentIndex];
            return true;
        }
        return false;
    }
} // namespace DebugUIWidgets
