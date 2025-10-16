#include "IScene.hpp"

#include "SceneSwitcher.hpp"

bool IScene::IsProgress() const {
    return progress_;
}

void IScene::Awake() {
    progress_ = true;
}

void IScene::Change() {
	if (!switcher_) return;
    if (next_.empty()) return;
    switcher_->Change(next_);
	progress_ = false;
}
