#include "include/Framework.hpp"
#include "include/IGame.hpp"

Framework::Framework() {
	engine_ = std::make_unique<Engine>();
	engine_->Initialize();
}

void Framework::Execute(std::unique_ptr<IGame> _game) {
	game_ = std::move(_game);
	this->Initialize();

	while (Loop()){
		// Main loop
		if (!game_)break;
		game_->Update();
	}
}

void Framework::Initialize() const {
	if (!game_)return;
	game_->Initialize();
}

bool Framework::Loop() const {
	//if (!engine_)return false;
	//if (!engine_->IsEnabled())return false;
	//engine_->Update();
	return engine_->IsEnabled();
}

void Framework::Shutdown() {
	if (game_){
		game_->Shutdown();
		game_.reset();
	}
	if (!engine_)return;
	engine_->Shutdown();
	engine_.reset();
}
