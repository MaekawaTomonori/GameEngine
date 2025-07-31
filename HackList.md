# Hack List - リファクタリングが必要な箇所

このドキュメントは、プロジェクト内でリファクタリングが必要な箇所をまとめたものです。

## 1. PostProcessExecutor - 未実装機能

### ファイル: `src/PostProcess/Executor/PostProcessExecutor.cpp`

**問題箇所:**
- 行122: `// SRVを設定（TODO: 実際のSRVが作成されたら有効化）`
- 行166: `// TODO: 実際にはSRVManagerを使用してSRVを作成し、sceneSRV_に設定`
- 行178-179: CopyImgシェーダーのPSOとRootSignature設定が未実装

**問題内容:**
- PostProcessExecutorでのSRV（Shader Resource View）作成と設定が未実装
- シーンテクスチャの適切なSRV設定が行われていない
- CopyImgシェーダーの完全な実装が不完全

**必要な変更:**
1. SRVManagerとの統合を実装
2. sceneSRV_の適切な初期化と設定
3. CopyImgシェーダーのPSOとRootSignatureの完全実装
4. Draw()メソッドでの適切なSRV設定

## 2. Line クラス - ハードコードされたMAX_LINES

### ファイル: `include/Line.hpp` 行39

**問題箇所:**
```cpp
const uint32_t MAX_LINES = 1000;
```

**問題内容:**
- 最大ライン数がハードコードされている
- 動的なサイズ変更に対応していない
- メモリ使用量の最適化ができない

**必要な変更:**
1. MAX_LINESを設定可能なパラメータに変更
2. 動的リサイズ機能の実装
3. エンジン設定ファイルまたはコンストラクタパラメータでの設定可能化
4. メモリプールの動的管理実装

## 3. LightManager - ハードコードされたライト数制限

### ファイル: `src/Light/LightManager.hpp` 行38

**問題箇所:**
```cpp
const LightCount MAX_COUNT {20, 20, 20};
```

**問題内容:**
- 各ライトタイプの最大数がハードコードされている
- パフォーマンステストや用途に応じた調整ができない
- GPU メモリ使用量の最適化が困難

**必要な変更:**
1. 設定ファイルからのライト数制限読み込み
2. 実行時でのライト数制限変更機能
3. ライトタイプ別の個別設定機能
4. GPU性能に応じた自動調整機能

## 4. Spriteクラス - マジックナンバーの使用

### ファイル: `src/Sprite/Sprite.cpp`

**問題箇所:**
- 行73: `size_ = {100, 100};` - デフォルトサイズがハードコード
- 行117: `0, 100.f` - 射影行列のnear/farプレーンがハードコード

**問題内容:**
- Spriteのデフォルトサイズが固定値
- 射影行列パラメータがハードコード
- スケーラビリティと保守性の問題

**必要な変更:**
1. デフォルト値を設定クラスまたは定数定義に移動
2. SpriteCommonでの統一的なデフォルト値管理
3. 射影行列パラメータの設定可能化
4. スケール対応のデフォルト値計算

## 5. Singleton パターンの過剰使用

### ファイル: 複数ファイル（Framework.cpp, Line.cpp等）

**問題箇所:**
```cpp
Singleton<TextureManager>::GetInstance()
Singleton<CameraManager>::GetInstance()
Singleton<SpriteCommon>::GetInstance()
// 他多数...
```

**問題内容:**
- 過度なSingletonパターンの使用によるテスタビリティの低下
- 依存関係の隠蔽と結合度の増加
- マルチスレッド環境での潜在的問題

**必要な変更:**
1. 依存性注入（DI）パターンの導入
2. ServiceLocatorパターンの実装
3. Singletonを必要最小限のクラスに限定
4. インターフェース分離とMockingサポート

## 6. カメラクラス - ハードコードされた制限値

### ファイル: `src/Camera/Camera.cpp`, `src/Camera/Camera.hpp`

**問題箇所:**
- Camera.hpp行29: `float far_ = 100.0f;` - デフォルトfarプレーン
- Camera.cpp行34-35: ImGuiの範囲設定がハードコード

**問題内容:**
- カメラのニア/ファープレーンのデフォルト値がハードコード
- デバッグUI範囲が固定値

**必要な変更:**
1. カメラ設定の外部設定ファイル化
2. シーンの規模に応じた自動調整機能
3. デバッグUI範囲の動的計算
4. カメラプリセット機能の実装

## 7. TextureManagerでの手動ResourceBarrier残存

### ファイル: `src/Texture/TextureManager.cpp` 行89-96

**問題箇所:**
```cpp
D3D12_RESOURCE_BARRIER barrier {};
// 手動でResourceBarrierを設定
```

**問題内容:**
- DX12Resourceラッパーが利用されていない箇所が残存
- DirectXTexライブラリとの整合性問題

**必要な変更:**
1. DirectXTexとDX12Resourceラッパーの統合アプローチ検討
2. ラッパー関数の実装による統一的なリソース管理
3. TextureManager専用のResourceBarrierラッパー実装

## 優先度

1. **高**: PostProcessExecutorの未実装機能（機能的に重要）
2. **中**: ハードコードされた値の設定化（保守性向上）
3. **中**: Singletonパターンの見直し（アーキテクチャ改善）
4. **低**: TextureManagerの統合（一貫性向上）

## 実装ガイドライン

- 設定値は`Config`クラスまたは専用の設定ファイルで管理
- ハードコードされた値は定数として明確に定義
- Singletonの代替として依存性注入を検討
- パフォーマンスクリティカルな箇所は慎重にリファクタリング