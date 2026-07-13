# Engine Issues & Task List

最終更新: 2026-06-12  
対象: PortfolioGame Engine (C++20 / DirectX12)

---

## 目次

### カテゴリA — アーキテクチャ設計の問題
| # | タイトル | 緊急度 |
|---|---------|--------|
| [A-1](#a-1-rendererの描画キュー分割設計) | Rendererの描画キュー分割設計 | 高 |
| [A-2](#a-2-uiのspritetext描画順序固定) | UIのSprite/Text描画順序固定 | 高 |
| [A-3](#a-3-layercanvasシステム未実装) | Layer/Canvasシステム未実装 | 高 |
| [A-4](#a-4-modelのskinclusternon-owning化) | ModelのSkinClusterがnon-skinでも常に確保 | 中 |
| [A-5](#a-5-singletonの過剰使用) | Singletonの過剰使用 | 中 |
| [A-6](#a-6-frameworkcheckの冗長なnullptrチェック) | Framework::Check()の冗長なnullptrチェック | 低 |

### カテゴリB — 機能不足・未実装
| # | タイトル | 緊急度 |
|---|---------|--------|
| [B-1](#b-1-postprocessexecutor-srvが未実装) | PostProcessExecutor SRVが未実装 | 高 |
| [B-2](#b-2-gpu-particle未実装) | GPU Particle未実装 | 中 |
| [B-3](#b-3-d3dresourceleakchecker-ifdef漏れ) | D3DResourceLeakCheckerの#ifdef漏れ | 高 |

### カテゴリC — コーディングルール違反
| # | タイトル | 件数 | 緊急度 |
|---|---------|------|--------|
| [C-1](#c-1-ヘッダー拡張子h禁止違反) | ヘッダー拡張子`.h`禁止違反 | 6件 | 低 |
| [C-2](#c-2-pragma-once禁止違反) | `#pragma once`禁止違反 | 7件 | 低 |
| [C-3](#c-3-インクルードガード命名規則違反-all_upper未遵守) | インクルードガード命名（ALL_UPPER未遵守） | 74件 | 低 |
| [C-4](#c-4-インクルードガードのスペルミス) | インクルードガードのスペルミス | 4件 | 中 |
| [C-5](#c-5-コメントスタイル違反) | コメントスタイル違反（`//` / `///` / `/** */`混在） | 479件 | 低 |
| [C-6](#c-6-assertの使用禁止違反) | `assert()`使用禁止違反（`Utils::Alert`使用ルール） | 29件 | 中 |

### カテゴリD — 命名・APIの問題
| # | タイトル | 緊急度 |
|---|---------|--------|
| [D-1](#d-1-iposteffectsetupの表記ゆれ) | `IPostEffect::SetUp()`の表記ゆれ | 低 |
| [D-2](#d-2-iposteffectmodifier名が動詞でない) | `IPostEffect::Modifier()`名が動詞でない | 低 |
| [D-3](#d-3-commonクラス名が抽象的すぎる) | `Common`クラス名が抽象的すぎる | 低 |
| [D-4](#d-4-winappとwindowの境界が不明確) | `WinApp`と`Window`の境界が不明確 | 低 |
| [D-5](#d-5-各所のあいまいな関数名) | 各所のあいまいな関数名 | 低 |
| [D-6](#d-6-lightの型別setメソッド名が抽象的) | Lightの型別`Set(...)`メソッド名が抽象的 | 低 |
| [D-7](#d-7-stagelodaderrecursive名が実装詳細を表す) | `StageLoader::Recursive()`名が実装詳細を表す | 低 |

### カテゴリE — ハードコード値
| # | タイトル | 緊急度 |
|---|---------|--------|
| [E-1](#e-1-max_lines--1000-linehpp) | `MAX_LINES = 1000` (Line.hpp) | 中 |
| [E-2](#e-2-max_count-202020-lightmanager) | `MAX_COUNT {20,20,20}` (LightManager) | 中 |
| [E-3](#e-3-spriteのデフォルトsize--100-100) | Spriteのデフォルト`size_ = {100, 100}` | 中 |
| [E-4](#e-4-cameraのnearfar-fovデフォルト値) | Cameraのnear/far/fovデフォルト値 | 中 |
| [E-5](#e-5-srvmanagerのkmaxsrvcount--512) | `SRVManager::kMaxSRVCount = 512` | 中 |
| [E-6](#e-6-particlesystem-pool_size--64) | `ParticleSystem::POOL_SIZE = 64` | 低 |

### カテゴリF — コード品質・DRY違反
| # | タイトル | 緊急度 |
|---|---------|--------|
| [F-1](#f-1-pso初期化パターンの重複) | PSO初期化パターンの重複（9箇所以上） | 中 |
| [F-2](#f-2-texturemanagerのresourcebarrier手動記述) | TextureManagerのResourceBarrier手動記述 | 低 |
| [F-3](#f-3-関数が長すぎる) | 関数が長すぎる（80行超え） | 低 |
| [F-4](#f-4-大きすぎるクラス) | 大きすぎるクラス（責務集中） | 低 |

---

## 詳細説明

---

### A-1: Rendererの描画キュー分割設計

**ファイル**: `src/Renderer/Renderer.hpp`, `src/Renderer/Renderer.cpp`

現在の`Renderer`は3つのキューを持ち、描画タスクを「ポストエフェクトあり / なし / UI」に事前分類している。

```cpp
std::queue<std::function<void()>> pp_;        // ポストエフェクトあり
std::queue<std::function<void()>> tasks_;     // ポストエフェクトなし
std::queue<std::function<void()>> uiTasks_;   // 常にswapchain直描画
```

**問題点**:
- タスクを積む時点で「どのレイヤーか」を2択（bool）で決定しなければならない
- 後から「このオブジェクトだけ別エフェクト」という変更が不可能
- `captureMode`（SceneView）の条件分岐が複雑化している
- ポストエフェクトの「マスク」を追加しようとした際（`Common.hpp`の`/** Add PostEffect Mask later */`コメント）に設計が破綻する

**理想的な設計方向（[A-3](#a-3-layercanvasシステム未実装)との統合）**:

```
RenderLayer 0  (RenderTexture_A) → PostEffect X → composite
RenderLayer 1  (RenderTexture_B) → PostEffect Y → composite
RenderLayer 2  (swapchain直)     → なし
```

タスク登録時に「レイヤー番号」を指定するだけで、エフェクトの適用対象が自動的に決まる。

---

### A-2: UIのSprite/Text描画順序固定

**ファイル**: `src/Framework/Framework.cpp:256-270`, `src/Common/Common.cpp:42-76`

`Framework::Draw()`が`sprite_->Draw(renderer_)` → `text_->Draw(renderer_)`の順で固定呼び出しをしている。`Common::Draw()`の実装を見ると、SpriteCommonのすべての描画コマンドを1つのRendererタスクにまとめてから`pipeline_->DrawCall()`を1回呼ぶ構造になっている。

```
実際の描画順序（固定）:
 1. 全モデル
 2. 全パーティクル
 3. 全ライン
 4. 全Sprite（SpriteCommonの全インスタンス一括）
 5. 全Text（TextCommonの全インスタンス一括）
```

このため `sprite0 → text0 → sprite1 → text1` のような交互描画は不可能。UIやゲーム内テキストのZ順序制御に直接影響する。

**対処案**:
- `RenderingCommand`に`int sortKey`を追加し、`Common::Draw()`内でソートしてから登録
- 同一PSOのコマンドはまとめて送ることでPSO切り替えコストを最小化
- [A-3](#a-3-layercanvasシステム未実装)のLayerシステムで根本解決することが望ましい

---

### A-3: Layer/Canvasシステム未実装

**関連ファイル**: `src/Renderer/Renderer.hpp`, `src/PostProcess/Executor/PostProcessExecutor.hpp`

現状、ポストエフェクトはシーン全体に対して一括適用される。「このキャラクターだけグロー」「このUIだけブラー」といった特定オブジェクトへのエフェクト適用が不可能。

`Common.hpp:18`には`/** Add PostEffect Mask later */`というコメントが残っており、設計上の意図はあるが未着手。

**必要な設計変更**:
1. `Renderer`のキューを「レイヤーID → タスクリスト」のマップに変更
2. `PostProcessExecutor`が複数の`RenderTexture`を管理
3. 各レイヤーのエフェクトを順次適用後、最終的にswapchainへ合成
4. `A-1`・`A-2`の問題も同時解決できる

実装規模は2〜3週間程度。GPU Particleより前提条件が少なく先行着手しやすい。

---

### A-4: ModelのSkinClusterがnon-skinでも常に確保

**ファイル**: `include/Model.hpp:91`

```cpp
SkinCluster skinCluster_;  // OBJモデルでも常にメモリ確保される
std::optional<Skeleton> pose_;  // こちらはoptional
```

`pose_`は`std::optional`で適切に扱われているが、`SkinCluster`はoptionalではない。スキンなしモデル（OBJ等）では`skinCluster_`の各メンバが使われないまま確保される。

パブリックAPIは変更不要（`Initialize(name)`で自動判定する設計は維持できる）。`std::optional<SkinCluster>`に変更するだけで対応可能。工数は半日程度。

---

### A-5: Singletonの過剰使用

**ファイル**: `src/Framework/Framework.cpp`他多数

以下がSingletonで管理されている：
`Input`, `TextureManager`, `SpriteCommon`, `ModelCommon`, `LineCommon`, `SkyCommon`, `LightManager`, `CameraController`, `CameraDirector`, `Ui::Manager`, `TextCommon`, `Screen`, `RandomEngine`（13クラス以上）

**問題点**:
- Frameworkでの生ポインタキャッシュ（`input_`, `texture_`等）と二重アクセスルートが存在する
- テスタビリティが低い（モック化困難）
- どのシステムが誰に依存しているかが隠蔽される

**段階的解消方針**（既存のREFACTORING_TODO.md Phase 3と対応）:
1. ServiceLocatorを導入してFrameworkが明示的に登録・提供する
2. Frameworkのキャッシュポインタを参照型（`Input&`等）に変換してnullptrチェックを排除
3. 最終的にSingletonを5個以下に削減

---

### A-6: Framework::Check()の冗長なnullptrチェック

**ファイル**: `src/Framework/Framework.cpp:313-326`

`input_`, `texture_`, `sprite_`等のSingleton由来ポインタはコンストラクタで必ず初期化されるため、`Check()`内でのnullptrチェックは意味をなさない。`Check()`の意図が「システム全体が起動済みか」なのか「重大なポインタがnullでないか」なのかが曖昧になっている。

また関数名`Check()`自体も何をチェックするかが不明瞭（`IsSystemReady()`や`CanContinueMainLoop()`が適切）。

---

### B-1: PostProcessExecutor SRVが未実装

**ファイル**: `src/PostProcess/Executor/PostProcessExecutor.cpp:122, 166, 178-179`

SRV（Shader Resource View）作成部分とCopyImgシェーダーのPSOが未実装のままTODOコメントが残っている。現状でポストエフェクトが正しく機能していない可能性がある。

対象箇所:
- 行122: `// SRVを設定（TODO: 実際のSRVが作成されたら有効化）`
- 行166: `// TODO: 実際にはSRVManagerを使用してSRVを作成し、sceneSRV_に設定`
- 行178-179: CopyImgシェーダーのPSOとRootSignature設定が未実装

機能上の優先度は高く、早急に対処すべき。

---

### B-2: GPU Particle未実装

**ファイル**: `src/ParticleSystem/`全体

現在のパーティクルシステムはCPU側で全計算をしてからGPUにアップロードする方式。大量パーティクル（数万以上）は処理できない。

**実装に必要な要素**:
- Compute Shader（`.CS.hlsl`）パイプラインの追加
- UAVバッファ（パーティクル状態の読み書き）
- `ExecuteIndirect`による間接描画
- 既存の`PrimitiveType`列挙（Billboard/Ring/Cylinder/Trail）は流用可能

工数は3〜4週間程度で最大規模の変更。Compute Shaderの基盤整備が先に必要となるため、他の機能追加が落ち着いてから着手が現実的。

---

### B-3: D3DResourceLeakCheckerの#ifdef漏れ

**ファイル**: `src/DirectX/DirectXAdapter.cpp:67`

```cpp
D3DResourceLeakChecker _lc;  // Releaseビルドでも有効になってしまっている
```

修正:
```cpp
#ifdef _DEBUG
    D3DResourceLeakChecker _lc;
#endif
```

30分以内に対応可能な最小変更。

---

### C-1: ヘッダー拡張子`.h`禁止違反

**ルール**: コーディングルール「ヘッダーファイルは`.hpp`を使用、`.h`は使用しない」

**違反ファイル（6件）**:
- `src/DirectX/Heap/SRVManager.h`
- `src/DirectX/Shader/Shader.h`
- `src/Light/DirectionalLight/DirectionalLight.h`
- `src/Light/PointLight/PointLight.h`
- `src/Light/RawLight.h`
- `src/Light/SpotLight/SpotLight.h`

ファイルリネーム + インクルードパス更新 + vcxproj更新。機械的作業で1〜2時間。

---

### C-2: `#pragma once`禁止違反

**ルール**: コーディングルール「`#pragma once`ではなく従来型インクルードガードを使用」

**違反ファイル（7件）**:
- `src/Collision/CollisionManager.hpp`
- `src/DirectX/Heap/SRVManager.h`
- `src/DirectX/Shader/Shader.h`
- `src/Light/DirectionalLight/DirectionalLight.h`
- `src/Light/PointLight/PointLight.h`
- `src/Light/RawLight.h`
- `src/Light/SpotLight/SpotLight.h`

C-1と同一ファイルが多いため、C-1と同時対応で解消できる。

---

### C-3: インクルードガード命名規則違反（ALL_UPPER未遵守）

**ルール**: `#ifndef [FILE_NAME]_HPP_`（ファイル名部分はすべて大文字）

**違反数**: 74件（`include/`および`src/`以下のほぼ全ファイル）

例:
```cpp
// 現状（誤り）
#ifndef Framework_HPP_

// 正しい形式
#ifndef FRAMEWORK_HPP_
```

量が多いが機械的置換で対応可能。正規表現で一括変換できる。

---

### C-4: インクルードガードのスペルミス

**ルール**: ガード名はファイル名と対応していること

**違反箇所（4件）**:
- `src/DirectX/DirectXAdapter.hpp:239` — ガード名が`DirectXAdaptor_HPP_`（Adaptor は誤字）
- `src/Mesh/Data/MeshData.hpp:22` — ガード名が`MESH_DATA_HPP_`（形式は正しいが内容と不整合）
- `src/Mesh/Repository/MeshRepository.hpp:22` — ガード名が`MeshLoader_HPP_`（ファイル名と不一致）
- `src/Model/Loader/ObjLoader.hpp:1` — ガード名が`ModelLoader_HPP_`（ファイル名と不一致）

スペルミスはビルド上の問題はないが、混乱の原因になる。

---

### C-5: コメントスタイル違反

**ルール**: Doxygenスタイルは`/** @brief ... */`を使用。`//`・`///`は使わない

**違反数**: 479件（`double-asterisk-block-start: 374`件、`double-slash: 102`件、`triple-slash: 3`件）

現状ほぼ全ヘッダーが`/**`を使用しており、ルール側を`/**`許容に改定するか、全ファイルを修正するかの方針決定が必要。

ルールを確認すると「`/**` スタイルを使用（`///`は使用しない）」とあるので、`/**`は正しく`//`のみ違反対象と読める。  
→ `//`コメント102件と`///`コメント3件が実際の違反。

---

### C-6: `assert()`使用禁止違反

**ルール**: 「標準`assert`ではなく`Utils::Alert`を使用」

**違反数**: 29件（`DirectXAdapter.cpp`、`TextureManager.cpp`他）

`assert()`はReleaseビルドで消えてしまい、致命的エラーが無音で通過する。`Utils::Alert()`に移行することで、ReleaseビルドでもLog出力とアプリ停止が保証される。

---

### D-1: `IPostEffect::SetUp()`の表記ゆれ

**ファイル**: `src/PostProcess/IPostEffect.hpp:33`

エンジン全体で`Setup`（小文字u）に統一されているが、`IPostEffect`のみ`SetUp`（大文字U）。派生クラス実装時にタイポの原因になる。

---

### D-2: `IPostEffect::Modifier()`名が動詞でない

**ファイル**: `src/PostProcess/IPostEffect.hpp:79`

```cpp
virtual void Modifier() = 0;  // 何をするのかわからない
```

コーディングルール「関数名はUpperCamelCase」は満たしているが、名前が動詞ではなく意味が不明瞭。
→ `ApplyParameters()`、`UpdateConstants()`、`RenderPass()`等が適切。

---

### D-3: `Common`クラス名が抽象的すぎる

**ファイル**: `src/Common/Common.hpp`

`Common`というクラス名は役割を説明していない。実態は「レンダリングサブシステムの基底クラス（更新/デバッグ/描画のコマンドレジストリを持つ）」。  
→ `RenderSubsystemBase`や`EngineModuleBase`が実態に即している。

---

### D-4: `WinApp`と`Window`の境界が不明確

**ファイル**: `src/Platform/WinApp.hpp`, `src/Window/Window.hpp`

同様の名前で2クラスが存在し、どちらが何を担当するかが名前から判断できない。
→ `PlatformWindowHost`（OSウィンドウの作成・管理）と`Window`（ゲームウィンドウの抽象）のように役割を名前に反映させる。

---

### D-5: 各所のあいまいな関数名

以下はコーディングルール上「関数は何をするかが名前から分かること」に違反している。

| 場所 | 現在の名前 | 推奨名 |
|------|-----------|--------|
| `include/Framework.hpp:97` | `Check()` | `IsSystemReady()` / `CanContinueMainLoop()` |
| `src/Timer/Timer.hpp:18` | `Check()` | `HasElapsed()` / `IsExpired()` |
| `src/Scene/SceneSwitcher.hpp:64` | `Change(name)` | `ChangeScene(name)` / `SwitchToScene(name)` |
| `src/Camera/Director/CameraDirector.hpp:80` | `Load(key)` | `LoadWorkByKey(key)` |
| `src/Camera/Director/CameraDirector.hpp:81` | `Run(key)` | `Play(key)` / `StartWork(key)` |
| `src/Light/LightManager.hpp:77` | `Add(type)` | `AddLight(type)` / `CreateLight(type)` |
| `src/Scheduler/Scheduler.hpp:16` | `RunTaskTimer(task, ms)` | `ScheduleRecurring(task, ms)` |
| `include/IScene.hpp:98` | `PostEffect()` | `GetPostProcessExecutor()` |
| `include/IScene.hpp:99` | `Particle()` | `GetParticleSystem()` |

---

### D-6: Lightの型別`Set(...)`メソッド名が抽象的

**ファイル**: `src/Light/PointLight/PointLight.h:25`, `SpotLight.h:34`, `DirectionalLight.h:26`

各Lightクラスに`Set(...)`メソッドがあるが引数の意味がクラスごとに異なり、名前から意図が読み取れない。  
→ `SetFromData()`、`ApplyLightParameters()`、または型固有の`SetPointLightData()`等が適切。

---

### D-7: `StageLoader::Recursive()`名が実装詳細を表す

**ファイル**: `src/Stage/Loader/StageLoader.hpp:18`

```cpp
static std::unique_ptr<LevelData> Recursive(const nlohmann::json& _base);
```

「再帰する」という実装手段を名前にしており、ドメイン的な意味がわからない。  
→ `ParseLevelDataTree()`、`BuildLevelDataTree()`等が適切。

---

### E-1: `MAX_LINES = 1000` (Line.hpp)

**ファイル**: `include/Line.hpp:39`

デバッグ描画の最大ライン数が`const uint32_t MAX_LINES = 1000`でハードコード。スケルトン描画やコリジョン可視化のコスト次第で不足する可能性がある。  
`EngineConfig`（P2-1）に統合して外部設定可能にすることが望ましい。

---

### E-2: `MAX_COUNT {20,20,20}` (LightManager)

**ファイル**: `src/Light/LightManager.hpp:38`

```cpp
const LightCount MAX_COUNT {20, 20, 20};  // Directional, Point, Spot
```

ライト数の上限がハードコードされており、シーンのスケール変更時に毎回コードを変更する必要がある。GPU構造体サイズに直結するため、変更はGPUバッファの再生成が必要。`EngineConfig`で管理するのが適切。

---

### E-3: Spriteのデフォルト`size_ = {100, 100}`

**ファイル**: `src/Sprite/Sprite.cpp:69`

```cpp
size_ = {100, 100};  // マジックナンバー
```

また`Sprite::UpdateMapData()`内の射影行列パラメータ`0, 100.f`（near/far）も同様。`EngineConfig`のSpriteDefaults節に統合すべき。

---

### E-4: Cameraのnear/far/fovデフォルト値

**ファイル**: `src/Camera/Camera.hpp:29`

```cpp
float far_ = 100.0f;  // デフォルトfarプレーン
```

near/far/fovのデフォルトがハードコード。シーンのスケールによって変更が必要になる。`EngineConfig`のCameraDefaults節で管理すべき。

---

### E-5: `SRVManager::kMaxSRVCount = 512`

**ファイル**: `src/DirectX/Heap/SRVManager.h`

SRVヒープの最大エントリ数が固定。テクスチャ・パーティクル・ポストエフェクト・シャドウマップ等の合計が増えると超過する。また`SRVManager::Allocate()`のバウンドチェックが`useIndex_ <= kMaxSRVCount`（等号を含む）でインクリメント後に境界を超えるオフバイワンの可能性がある（要確認）。

---

### E-6: `ParticleSystem::POOL_SIZE = 64`

**ファイル**: `src/ParticleSystem/ParticleSystem.hpp:52`

エミッタープールの上限数がハードコード。大量のパーティクルエフェクトを同時使用するシーンで制限になる。

---

### F-1: PSO初期化パターンの重複

**ファイル**: `src/Model/Common/ModelCommon.cpp`, `src/PostProcess/BoxBlur/BoxBlur.cpp`, `src/PostProcess/Grayscale/Grayscale.cpp`, `src/PostProcess/Vignette/Vignette.cpp`, `src/Sky/Common/SkyCommon.cpp`他

PipelineStateObject初期化の5行パターンが9箇所以上に重複している（`coderule_refactor_additional.md` Cluster 2〜6参照）。`Common`基底クラスにテンプレートメソッドとして抽出できる。

---

### F-2: TextureManagerのResourceBarrier手動記述

**ファイル**: `src/Texture/TextureManager.cpp:89-96`

`DX12Resource`ラッパーが存在するにも関わらず、`D3D12_RESOURCE_BARRIER`を手動設定しているコードが残存。`DX12Resource::TransitionBarrier()`を実装してラッパーを使用すべき。

---

### F-3: 関数が長すぎる

**ルール**: 関数は単一の責務を持ち、読める長さであること

**80行超えの関数（主要なもの）**:

| ファイル | 関数 | 行数 |
|---------|------|------|
| `PostProcessPresetEditor.cpp:58` | `RenderAvailablePresetsSection()` | 152行 |
| `PostProcessPresetEditor.cpp:304` | `RenderMembersList()` | 122行 |
| `ParticleSystem.cpp:271` | `Debug()` | 121行 |
| `Log.cpp:280` | `Debug()` | 120行 |
| `CameraDirector.cpp:481` | `ShowEditor()` | 170行 |
| `PostProcessExecutor.cpp:400` | `ApplyPreset()` | 112行 |
| `SceneSwitcher.cpp:92` | `Debug()` | 100行 |

Debug系関数はUI描画ロジックの断片をまとめてプライベートメソッドに切り出すことで解消できる。

---

### F-4: 大きすぎるクラス

**責務が集中しているクラス**:

| クラス | ファイル | 説明 |
|--------|---------|------|
| `PostProcessPresetEditor` | PostProcessPresetEditor.cpp (1172行) | リストUI / エディタフォーム / キーフレームパネル / シリアライズに分割可能 |
| `PostProcessExecutor` | PostProcessExecutor.cpp (504行) | プリセット遷移スケジューラ / パス実行パイプラインに分割可能 |
| `CameraDirector` | CameraDirector.cpp (635行) | ランタイム補間エンジン / デバッグUIに分割可能 |
| `DirectXAdapter` | DirectXAdapter.cpp (616行) | デバイス管理 / スワップチェーン / コマンドキューに分割可能 |
| `Model` | Model.cpp (414行) | スキニング処理をSkinAnimatorクラスとして分離可能 |

---

## 推奨対応順序

```
今週（30分〜1日）:
  B-3  D3DResourceLeakChecker #ifdef   ← 30分
  C-1  .h → .hpp リネーム              ← 2時間（C-2も同時解消）
  C-4  ガードのスペルミス修正           ← 1時間

来週（1〜3日）:
  B-1  PostProcessExecutor SRV実装     ← 最優先（機能不全の可能性）
  A-4  SkinCluster optional化          ← 半日
  E-1〜E-4  ハードコード値の設定化      ← EngineConfig統合後

今月（1〜2週間）:
  A-2  Sprite/Text描画順序修正
  C-6  assert() → Utils::Alert()
  C-3  ガード命名 ALL_UPPER化（一括変換）

中期（1〜2ヶ月）:
  A-3  Layer/Canvasシステム設計・実装
  A-5  Singleton段階的削減（ServiceLocator導入）
  F-1  PSO初期化共通化

長期（2ヶ月以上）:
  B-2  GPU Particle
  A-5  Singleton完全解消
  F-3/F-4  大関数・大クラスの分割
```
