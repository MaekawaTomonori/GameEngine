#ifndef Framework_HPP_
#define Framework_HPP_
#include <memory>

#include "Engine/Engine.hpp"
#include "IGame.hpp"

class Framework {
    std::unique_ptr<Engine> engine_;
    std::unique_ptr<IGame> game_;
public:
	Framework() = delete;
	Framework(std::unique_ptr<IGame> _game);
	void Run();
private:
	void Initialize();
	void Update();
	void Shutdown();
}; // class Framework

#endif // Framework_HPP_
