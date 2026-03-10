#include "Ui/EventSystem.hpp"

#include <ranges>

namespace Ui {
    void EventSystem::Register(const std::string& _actionKey, const std::function<void()>& _action) {
        if (!actions_.contains(_actionKey)) {
            actions_[_actionKey] = _action;
        }
    }

    void EventSystem::Execute(const std::string& _actionKey) {
        auto it = actions_.find(_actionKey);
        if (it != actions_.end() && it->second) {
            it->second();
        }
    }

    bool EventSystem::Has(const std::string& _actionKey) const {
        return actions_.contains(_actionKey);
    }

    std::vector<std::string> EventSystem::GetActionKeys() const {
        std::vector<std::string> keys;
        keys.reserve(actions_.size());
        for (const auto& key : actions_ | std::views::keys) {
            keys.push_back(key);
        }
        return keys;
    }

} // namespace Ui
