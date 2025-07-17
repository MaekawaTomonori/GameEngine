#ifndef RenderPhase_HPP_
#define RenderPhase_HPP_

enum class RenderPhase {
    Scene,      // シーン描画（PostEffect適用対象）
    PostEffect, // PostEffect処理
    UI          // UI描画（PostEffect非適用）
}; // enum class RenderPhase

#endif // RenderPhase_HPP_