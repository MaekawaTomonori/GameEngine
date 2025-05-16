#include "include/Framework.hpp"
#include "include/IGame.hpp"

Framework::Framework() {
    //engine_ = std::make_unique<Engine>();
    //engine_->Initialize();
    window_ = std::make_unique<Window>();
    window_->Create();
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

void Framework::Initialize() {
	if (game_){
		game_->Initialize();
	} else {
		(void)game_;
	}
}

bool Framework::Loop() const {
    //if (!engine_)return false;
	//if (!engine_->IsEnabled())return false;
    //engine_->Update();
    return window_->IsEnabled();
}

void Framework::Shutdown() {
}
