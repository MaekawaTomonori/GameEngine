# DESIGN.md

## Read first

1. design-tokens
2. component-architecture
3. component mapping
4. operation guide

## Source of truth

- Tokens SSoT: design tokens in JSON
- Component mapping: design-to-code mapping file

## Rules

- Do not introduce external UI vocabulary directly
- Normalize requested UI patterns before implementation
- Verify design-related changes with `pnpm penpot:verify`

---

## Engine Editor / Debugger Design Prompt

あなたはシニアゲームエンジンエディタ設計者です。
C++ / DirectX12ベースの自作ゲームエンジンに対して、
Unity Editor Foundationsを参考にしたDebuggerおよびUI設計を行ってください。

### Objectives

- デバッグ効率の最大化
- UIの一貫性維持
- 初見でも理解可能な操作性
- Engine内部構造の過剰露出を防ぐ
- 将来拡張に耐える構造

---

## design-tokens

### State Tokens
- state.normal
- state.hover
- state.active
- state.disabled
- state.selected

### Semantic Color Tokens
- color.info
- color.warning
- color.error
- color.success
- color.debug-highlight

### Spacing Tokens
- spacing.small
- spacing.medium
- spacing.large

### Typography Tokens
- label.short
- label.long
- debug.mono

---

## component-architecture

- Window
- Panel
- Inspector
- DebugPanel
- Overlay
- Control

---

## component mapping

### Button
- action.trigger
- primary / secondary / danger

### Dropdown
- mode.switch
- setting.select

### Foldout
- group.container
- debug.section

### ColorField
- material.color
- light.color

---

## operation guide

### Authoring Flow
1. オブジェクト選択
2. Inspector編集
3. Scene反映
4. Debug確認

### Debug Flow
1. 実行開始
2. 状態監視
3. 問題検出
4. 修正
5. 再実行

---

## Content Organization Rules

- グルーピングは余白ベース
- コンテナは操作性がある場合のみ使用
- 階層は視覚的に明確化

---

## Empty State Design

- 明確なメッセージ表示
- 次のアクション提示

---

## Error / Messaging

- Info / Warning / Error
- 原因 / 発生箇所 / 対処

---

## Debugger Requirements

- 再生 / 停止 / 一時停止
- フレームステップ
- 変数ウォッチ
- ログフィルタ
- GPU / CPU状態表示
- イベントトレース

---

## Window System

- Scene
- Game
- Hierarchy
- Inspector
- Console
- Profiler
- Debugger

---

## Internal Architecture

- EditorWindowManager
- DockLayoutSystem
- DebuggerSystem
- LogSystem
- SelectionService
- CommandHistory
- PropertyInspector
- ContextualToolSystem

---

## MVP Implementation Order

1. Window System
2. Inspector
3. Console
4. Debugger Core
5. Selection連携
6. Undo/Redo
7. Overlay

---

## Anti-patterns

- 情報過多UI
- 状態不明UI
- 色依存設計
- 不明確な操作要素
- コンテナ乱用
- Debug情報の未整理表示
- Engine内部の露出過多

---

## Output Requirement

- 実装可能レベル
- UI構造とデータフロー明確化
- クラス設計まで踏み込む
