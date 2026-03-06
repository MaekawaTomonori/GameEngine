#pragma once
#include <memory>

#include "DebugUI.hpp"
#include "Line.hpp"
#include "Collision/CollisionManager.h"

/** @brief Engine 側の衝突管理ラッパー
 ** Collision::Manager の呼び出しとデバッグ可視化を担当
 **/
class CollisionManager {
    Collision::Manager* manager_ = nullptr;

    DebugUI* debugUI_ = nullptr;

    std::unique_ptr<Line> debugLine_;
    bool debugEnabled_ = false;
public:
    void Initialize(DebugUI* _debugUI);
    void Update();
    void DrawDebug() const;
    void Debug();

private:
    void RebuildDebugLines();
    void DrawSphere(const Vector3& center, float radius);
    void DrawAABB(const Vector3& center, const Vector3& half);
};
