#include "CollisionManager.hpp"

#include <variant>

#include "Log.hpp"
#include "Utils.hpp"
#include "Pattern/Singleton.hpp"
#include "Math/MathUtils.hpp"

#ifdef _DEBUG
#include "imgui.h"
#endif

void CollisionManager::Initialize([[maybe_unused]]DebugUI* _debugUI) {
    manager_ = Singleton<Collision::Manager>::GetInstance();
#ifdef _DEBUG
    debugUI_ = _debugUI;
    if (!debugUI_) {
        Utils::Alert("CollisionManager: DebugUI not registered");
        return;
    }

    debugUI_->RegisterMenuButton("Collision");

    debugLine_ = std::make_unique<Line>();
    debugLine_->Initialize();
    debugLine_->SetName("CollisionDebug");
    debugLine_->SetColor({0.2f, 1.0f, 0.2f, 1.0f});
#endif
}


void CollisionManager::Update() {
    manager_->Detect();
    manager_->ProcessEvent();

#ifdef _DEBUG
    if (debugEnabled_) {
        RebuildDebugLines();
    }
#endif
}

void CollisionManager::DrawDebug() const {
#ifdef _DEBUG
    if (debugEnabled_) {
        debugLine_->Draw();
    }
#endif
}

void CollisionManager::Debug() {
#ifdef _DEBUG
    if (!debugUI_) return;

    debugUI_->RegisterCommand("Collision", [&](){
        ImGui::Begin("Collision", &debugUI_->IsVisible("Collision"));
        ImGui::Checkbox("Debug Lines", &debugEnabled_);

        const auto colliders = manager_->GetAll();
        ImGui::Text("Colliders: %d", static_cast<int>(colliders.size()));
        ImGui::End();
    });
#endif
}

void CollisionManager::RebuildDebugLines() {
    debugLine_->Clear();

    const auto colliders = manager_->GetAll();
    for (const auto* c : colliders) {
        if (!c->IsEnabled()) continue;

        const auto size = c->GetSize();
        const Vector3 center = c->GetTranslate();

        if (std::holds_alternative<float>(size)) {
            DrawSphere(center, std::get<float>(size));
        } else {
            const Vector3 half = std::get<Vector3>(size) * 0.5f;
            DrawAABB(center, half);
        }
    }

    debugLine_->Update();
}

void CollisionManager::DrawSphere(const Vector3& center, float radius) {
    constexpr int SEGMENTS = 16;
    constexpr float TAU = 2.0f * MathUtils::F_PI;

    // XY plane
    for (int i = 0; i < SEGMENTS; ++i) {
        float a0 = TAU * static_cast<float>(i)     / SEGMENTS;
        float a1 = TAU * static_cast<float>(i + 1) / SEGMENTS;
        Vector3 p0 = {center.x + radius * std::cosf(a0), center.y + radius * std::sinf(a0), center.z};
        Vector3 p1 = {center.x + radius * std::cosf(a1), center.y + radius * std::sinf(a1), center.z};
        debugLine_->AddLine(p0, p1);
    }

    // XZ plane
    for (int i = 0; i < SEGMENTS; ++i) {
        float a0 = TAU * static_cast<float>(i)     / SEGMENTS;
        float a1 = TAU * static_cast<float>(i + 1) / SEGMENTS;
        Vector3 p0 = {center.x + radius * std::cosf(a0), center.y, center.z + radius * std::sinf(a0)};
        Vector3 p1 = {center.x + radius * std::cosf(a1), center.y, center.z + radius * std::sinf(a1)};
        debugLine_->AddLine(p0, p1);
    }

    // YZ plane
    for (int i = 0; i < SEGMENTS; ++i) {
        float a0 = TAU * static_cast<float>(i)     / SEGMENTS;
        float a1 = TAU * static_cast<float>(i + 1) / SEGMENTS;
        Vector3 p0 = {center.x, center.y + radius * std::cosf(a0), center.z + radius * std::sinf(a0)};
        Vector3 p1 = {center.x, center.y + radius * std::cosf(a1), center.z + radius * std::sinf(a1)};
        debugLine_->AddLine(p0, p1);
    }
}

void CollisionManager::DrawAABB(const Vector3& center, const Vector3& half) {
    const Vector3 mn = center - half;
    const Vector3 mx = center + half;

    // Bottom face
    debugLine_->AddLine({mn.x, mn.y, mn.z}, {mx.x, mn.y, mn.z});
    debugLine_->AddLine({mx.x, mn.y, mn.z}, {mx.x, mn.y, mx.z});
    debugLine_->AddLine({mx.x, mn.y, mx.z}, {mn.x, mn.y, mx.z});
    debugLine_->AddLine({mn.x, mn.y, mx.z}, {mn.x, mn.y, mn.z});

    // Top face
    debugLine_->AddLine({mn.x, mx.y, mn.z}, {mx.x, mx.y, mn.z});
    debugLine_->AddLine({mx.x, mx.y, mn.z}, {mx.x, mx.y, mx.z});
    debugLine_->AddLine({mx.x, mx.y, mx.z}, {mn.x, mx.y, mx.z});
    debugLine_->AddLine({mn.x, mx.y, mx.z}, {mn.x, mx.y, mn.z});

    // Vertical edges
    debugLine_->AddLine({mn.x, mn.y, mn.z}, {mn.x, mx.y, mn.z});
    debugLine_->AddLine({mx.x, mn.y, mn.z}, {mx.x, mx.y, mn.z});
    debugLine_->AddLine({mx.x, mn.y, mx.z}, {mx.x, mx.y, mx.z});
    debugLine_->AddLine({mn.x, mn.y, mx.z}, {mn.x, mx.y, mx.z});
}
