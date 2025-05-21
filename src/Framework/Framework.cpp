#include "include/Framework.hpp"
#include "include/IGame.hpp"
#include "src/Timer/Timer.hpp"

Framework::Framework() {
	engine_ = std::make_unique<Engine>();
	engine_->Initialize();
}

void Framework::Execute(std::unique_ptr<IGame> _game) {
	game_ = std::move(_game);
	Initialize();

	Timer timer(std::chrono::milliseconds(50));
	timer.Start();
	while (Loop()){
		// Main loop
		if (!game_)break;

		if (timer.Check()){
			game_->Update();
			timer.Restart();
		}

		game_->Draw();
	}

	Shutdown();
}

void Framework::Initialize() const {
	if (!game_)return;
	game_->Initialize();
}

bool Framework::Loop() const {
	if (!engine_)return false;
	if (!engine_->IsEnabled())return false;

	engine_->Update();
	return true;
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
