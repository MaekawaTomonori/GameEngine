#ifndef Framework_HPP_
#define Framework_HPP_

#include <memory>
#include "src/Engine/Engine.hpp"
#include "src/Engine/Config/Config.hpp"

class IGame;

class Framework {
    GameEngine::Config* config_ = nullptr;

    std::unique_ptr<Engine> engine_;
    std::unique_ptr<IGame> game_;
public:
    Framework();

	void Execute(std::unique_ptr<IGame> _game){
        game_ = std::move(_game);
        Initialize();

        while (Loop()){
            Update();
            Draw();
        }

        Shutdown();
    }

private:
    void Initialize();
    bool Loop() const;
    void Update() const;
    void Draw() const;
    void Shutdown();
}; // class Framework

#endif // Framework_HPP_
