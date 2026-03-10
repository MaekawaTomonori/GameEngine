#ifndef EventSystem_HPP_
#define EventSystem_HPP_
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Ui {

    enum class EventKey {
        Execute,
        Click,
        Hover,
    };

    inline const char* EventKeyToString(EventKey _key) {
        switch (_key) {
            case EventKey::Execute: return "Execute";
            case EventKey::Click:   return "Click";
            case EventKey::Hover:   return "Hover";
        }
        return "Unknown";
    }

    inline EventKey StringToEventKey(const std::string& _str) {
        if (_str == "Execute") return EventKey::Execute;
        if (_str == "Click")   return EventKey::Click;
        if (_str == "Hover")   return EventKey::Hover;
        return EventKey::Execute;
    }

    constexpr EventKey ALL_EVENT_KEYS[] = {
        EventKey::Execute,
        EventKey::Click,
        EventKey::Hover,
    };

    class EventSystem {
        std::unordered_map<std::string, std::function<void()>> actions_;

    public:
        void Register(const std::string& _actionKey, const std::function<void()>& _action);
        void Execute(const std::string& _actionKey);
        bool Has(const std::string& _actionKey) const;
        std::vector<std::string> GetActionKeys() const;
    };

} // namespace Ui

template<>
struct std::hash<Ui::EventKey> {
    size_t operator()(Ui::EventKey _key) const noexcept {
        return std::hash<int>{}(static_cast<int>(_key));
    }
};

#endif // EventSystem_HPP_
