#include "Canvas.hpp"

#include <utility>

void Canvas::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv, const std::string& _name) {
    name_ = _name;
    chain_ = std::make_unique<PostEffectChain>();
    chain_->Initialize(_adapter, _srv, _name);
}

void Canvas::SetFactory(GESTD::ReferencePtr<PostEffectFactory> _factory) {
    chain_->SetFactory(_factory);
}

void Canvas::RegisterTask(std::function<void()> _task) {
    tasks_.push_back(std::move(_task));
}

void Canvas::DrawObjects() {
    chain_->BeginFrame();

    for (auto& task : tasks_) {
        task();
    }
    tasks_.clear();

    chain_->EndFrame();
}

void Canvas::ApplyPostEffects() {
    chain_->Execute();
}

void Canvas::Draw() const {
    chain_->Draw();
}
