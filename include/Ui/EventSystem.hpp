#ifndef EventSystem_HPP_
#define EventSystem_HPP_
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Ui {

    enum class EventKey {
        Hover,
        Execute,
    };

    inline const char* EventKeyToString(EventKey _key) {
        switch (_key) {
            case EventKey::Hover:   return "Hover";
            case EventKey::Execute: return "Execute";
        }
        return "Unknown";
    }

    inline EventKey StringToEventKey(const std::string& _str) {
        if (_str == "Hover")   return EventKey::Hover;
        if (_str == "Execute") return EventKey::Execute;
        return EventKey::Execute;
    }

    constexpr EventKey ALL_EVENT_KEYS[] = {
        EventKey::Hover,
        EventKey::Execute,
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
