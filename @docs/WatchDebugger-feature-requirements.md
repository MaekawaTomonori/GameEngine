# WatchDebugger 機能要件メモ

## 背景

GameStatus の `maxEnemyCount` / `requirementKill` (uint16_t)、`point` (uint32_t) を
ENGINE_WATCH_EDIT で扱おうとしたが、現状の対応型に含まれていないため未対応。

## 現状の対応型

`float` / `int32_t` / `bool` / `Vector2` / `Vector3` / `Vector4`

## 追加が必要な型

| 型 | 用途例 |
|----|--------|
| `uint16_t` | GameStatus::maxEnemyCount, requirementKill |
| `uint32_t` | GameStatus::point |

## 対応方針案

### A. 型サポートを直接追加
`EditType` enum に `Uint16` / `Uint32` を追加し、`WatchValue` variant・`DeduceType<T>()` を拡張。
ImGui 側は `DragScalar` (ImGuiDataType_U16 / U32) を使用。

### B. int32_t へのキャスト＋コールバック
`WatchEdit` のオーバーロードとして「読み書きコールバック版」を追加。
```cpp
ENGINE_WATCH_EDIT_CB("MaxEnemyCount",
    [&]() -> int32_t { return status_.maxEnemyCount; },
    [&](int32_t v)   { status_.maxEnemyCount = static_cast<uint16_t>(v); });
```
型変換をユーザー側で吸収するため、WatchDebugger 本体の変更が最小限。

## 優先度

現時点では uint 型は ENGINE_WATCH (読み取り専用ラムダ) で代替可能なため、
実装優先度は低。ゲームパラメータ調整の用途が増えたタイミングで対応する。
