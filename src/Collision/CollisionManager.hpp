#pragma once
#include <memory>

#include "ReferencePtr.hpp"
#include "DebugUI.hpp"
#include "Line.hpp"
#include "Collision/CollisionManager.h"

/** @brief Engine 側の衝突管理ラッパー
 * Collision::Manager の呼び出しとデバッグ可視化を担当
 */
class CollisionManager {
    GESTD::ReferencePtr<Collision::Manager> manager_;

    GESTD::ReferencePtr<DebugUI> debugUI_;

    std::unique_ptr<Line> debugLine_;
    bool debugEnabled_ = false;

public:
    void Initialize(const GESTD::ReferencePtr<DebugUI>& _debugUi);
    void Update();
    void DrawDebug() const;
    void Debug();

private:
    void RebuildDebugLines();
    void DrawSphere(const Vector3& center, float radius);
    void DrawAABB(const Vector3& center, const Vector3& half);
};
