#include "IScene.hpp"

#include "SceneSwitcher.hpp"

bool IScene::IsProgress() const {
    return progress_;
}

void IScene::Awake() {
    progress_ = true;
}

void IScene::Awake(SceneSwitcher *_switcher) {
	switcher_ = _switcher;
	progress_ = true;
}

void IScene::SetSwitcher(SceneSwitcher *_switcher) {
	switcher_ = _switcher;
}

void IScene::SetGame(IGame* _game) {
    game_ = _game;
}

void IScene::Change() {
	if (!switcher_) return;
    if (next_.empty()) return;
    switcher_->Change(next_);
	progress_ = false;
}
