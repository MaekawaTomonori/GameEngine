#ifndef Framework_HPP_
#define Framework_HPP_

#include <memory>
#include "src/Engine/Engine.hpp"

class IGame;

class Framework {
    //std::unique_ptr<Engine> engine_;
	std::unique_ptr<Window> window_;
    std::unique_ptr<IGame> game_;
public:
	Framework();
	void Execute(std::unique_ptr<IGame> _game);
private:
	void Initialize();
	bool Loop() const;
	void Shutdown();
}; // class Framework

#endif // Framework_HPP_
