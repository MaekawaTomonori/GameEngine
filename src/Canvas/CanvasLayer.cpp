#include "CanvasLayer.hpp"

#include "Log.hpp"

void CanvasLayer::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv) {
    adapter_ = _adapter;
    srv_ = _srv;
}

void CanvasLayer::SetFactory(GESTD::ReferencePtr<PostEffectFactory> _factory) {
    factory_ = _factory;
    for (auto& [zOrder, canvas] : canvases_) {
        canvas->SetFactory(factory_);
        canvas->GetChain()->LoadPermanentConfig();
    }
}

Canvas* CanvasLayer::AddCanvas(const std::string& _name) {
    ZOrder assignedZ = kAutoZBegin_;

    auto it = canvases_.upper_bound(kAutoZEnd_);
    if (it != canvases_.begin()) {
        --it;
        if (it->first >= kAutoZEnd_) {
            Log::Send(Log::Level::ERR, "CanvasLayer: auto Z-order range is full");
            return nullptr;
        }
        assignedZ = it->first + 1;
    }

    auto canvas = std::make_unique<Canvas>();
    canvas->Initialize(adapter_, srv_, _name);
    if (factory_) {
        canvas->SetFactory(factory_);
        canvas->GetChain()->LoadPermanentConfig();
    }

    Canvas* ptr = canvas.get();
    canvases_.emplace(assignedZ, std::move(canvas));
    return ptr;
}

Canvas* CanvasLayer::AddCanvasTop(const std::string& _name) {
    ZOrder assignedZ = kTopZBegin_;

    if (!canvases_.empty()) {
        auto it = canvases_.rbegin();
        if (it->first >= kTopZBegin_) {
            assignedZ = it->first + 1;
        }
    }

    auto canvas = std::make_unique<Canvas>();
    canvas->Initialize(adapter_, srv_, _name);
    if (factory_) {
        canvas->SetFactory(factory_);
        canvas->GetChain()->LoadPermanentConfig();
    }

    Canvas* ptr = canvas.get();
    canvases_.emplace(assignedZ, std::move(canvas));
    return ptr;
}

void CanvasLayer::RemoveCanvas(const std::string& _name) {
    for (auto it = canvases_.begin(); it != canvases_.end(); ++it) {
        if (it->second->GetName() == _name) {
            canvases_.erase(it);
            return;
        }
    }
}

Canvas* CanvasLayer::GetCanvas(const std::string& _name) const {
    for (auto& [zOrder, canvas] : canvases_) {
        if (canvas->GetName() == _name) {
            return canvas.get();
        }
    }
    return nullptr;
}

std::vector<std::pair<CanvasLayer::ZOrder, Canvas*>> CanvasLayer::GetCanvases() const {
    std::vector<std::pair<ZOrder, Canvas*>> result;
    result.reserve(canvases_.size());
    for (auto& [zOrder, canvas] : canvases_) {
        result.emplace_back(zOrder, canvas.get());
    }
    return result;
}

void CanvasLayer::DrawObjects() {
    for (auto& [zOrder, canvas] : canvases_) {
        if (canvas->IsEnabled()) {
            canvas->DrawObjects();
        }
    }
}

void CanvasLayer::ApplyPostEffects() {
    for (auto& [zOrder, canvas] : canvases_) {
        if (canvas->IsEnabled()) {
            canvas->ApplyPostEffects();
        }
    }
}

void CanvasLayer::DrawCanvases() const {
    for (auto& [zOrder, canvas] : canvases_) {
        if (canvas->IsEnabled()) {
            canvas->Draw();
        }
    }
}

void CanvasLayer::ResizeRenderTextures() const {
    for (auto& [zOrder, canvas] : canvases_) {
        canvas->ResizeRenderTextures();
    }
}
