#ifndef UiIModule_HPP_
#define UiIModule_HPP_

#include <string_view>
#include "json.hpp"

class Input;

namespace Ui {
    class Canvas;

    class IModule {
    public:
        virtual ~IModule() = default;
        virtual std::string_view GetType() const = 0;
        virtual void Update(Canvas& canvas, const Input& input) = 0;
        virtual void Draw() const {}
        virtual void LoadFromJson([[maybe_unused]] const nlohmann::json& j) {}
        virtual void SaveToJson([[maybe_unused]] nlohmann::json& j) const {}
    };

} // namespace Ui

#endif // UiIModule_HPP_
