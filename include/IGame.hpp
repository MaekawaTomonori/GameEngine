#ifndef IGame_HPP_
#define IGame_HPP_

#include <memory>

#include "src/Engine/Config/Config.hpp"

class IGame {
	std::unique_ptr<GameEngine::Config> config_;
public:
	IGame();
	virtual ~IGame() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Shutdown() = 0;
	
	GameEngine::Config& GetCurrentConfig() const;
}; // class IGame

#endif // IGame_HPP_
