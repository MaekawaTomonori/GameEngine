#ifndef UiElement_HPP_
#define UiElement_HPP_
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Sprite.hpp"
#include "Ui/EventSystem.hpp"

namespace Ui{
    class Element {
    public:
        struct Data{
            std::string texture = "white_x16.png";
            bool visible = true;
            Vector2 position {};
            Vector2 size{64.f, 64.f};
            Vector4 color{1.f, 1.f, 1.f, 1.f};
            Vector2 textureLeftTop{};
            Vector2 textureSize{};
        };

    private:
        std::string uuid_;
        std::string name_;

        std::unique_ptr<Sprite> sprite_ = nullptr;

        Vector2 parent_ {};

        Data data_{};

        std::unordered_map<EventKey, std::string> events_;

        bool isOpen_ = true;
        bool aspectLock_ = false;
        bool changeName_ = false;
        std::array<char, 256> nameBuff_ = {};
        std::array<char, 256> textureBuff_ = {};

    public:
        void Initialize();
        void Update();
        void Draw() const;

        void Debug(const std::vector<std::string>& _availableActions = {});

        bool IsVisible() const;
        std::string GetName() const;
        void SetName(const std::string& _name);
        std::string GetUUID() const;
        bool& IsOpen();

        Data GetData() const;
        void SetData(const Data& _data);

        void SetEvent(EventKey _event, const std::string& _actionKey);
        const std::string& GetActionKey(EventKey _event) const;
        bool HasEvent(EventKey _event) const;
        bool HasAnyEvent() const;
        const std::unordered_map<EventKey, std::string>& GetEvents() const;

        void SetParent(const Vector2& _pos);
    }; // class Element
} // namespace Ui

#endif // UiElement_HPP_
