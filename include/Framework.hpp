#ifndef Framework_HPP_
#define Framework_HPP_

#include <memory>
#include "Engine/Engine.hpp"
class IGame;

class Framework {
    std::unique_ptr<Engine> engine_;
    std::unique_ptr<IGame> game_;
public:
	Framework() = default;
	Framework(std::unique_ptr<IGame> _game);
	void Run();
private:
	void Initialize();
	void Update();
	void Shutdown();
}; // class Framework

#endif // Framework_HPP_
