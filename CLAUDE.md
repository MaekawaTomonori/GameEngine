# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## ファイル検索とアクセス規則

### アセットファイルの検索
- **Assetsファイル**と言及された場合は、`.\Assets\`フォルダから開始して関連するサブフォルダにアクセスしてください
  - シェーダー: `.\Assets\Shaders\`
  - テクスチャ: `.\Assets\Resources\`（PNG、JPGファイル）
  - モデル: `.\Assets\Resources\`（GLTF、OBJ、Blendファイル）
  - フォント: `.\Assets\Fonts\`
  - データ: `.\Assets\Data\`

### クラスとソースコードの検索
- **クラス、システム、実装**と言及された場合は、`.\src\`フォルダから開始して検索してください
- サブシステム別ディレクトリ構造に従ってナビゲートしてください

## プロジェクト概要

これはC++20 DirectX12ベースのゲームエンジンテンプレートのEngineコンポーネントです。Engineディレクトリには、レンダリング、入力処理、シーン管理、リソース読み込みなどの主要なゲームエンジン機能を提供する静的ライブラリ（.lib）が含まれています。

## 設計哲学

### Good Taste in Design
このエンジンは「実用性」と「明確性」を重視した設計になっています：

- **実用主義優先**: 理論的完璧性よりも、実際に動作する堅牢なコードを重視
- **DirectX12の複雑性管理**: GPUリソースの破棄順序や同期ポイントを明示的に制御
- **所有権の明確化**: `unique_ptr`と生ポインタの明確な使い分けによる意図的な所有権管理

### 重要な設計判断

#### 1. 所有権ルール（Ownership Rules）

このエンジンでは所有権を以下のように区別しています：

```cpp
// Framework.hpp
class Framework {
    // Frameworkが所有（削除責任あり）
    std::unique_ptr<DirectXAdapter> dxAdapter_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<ResourceRepository> resources_;

    // Singleton由来（Singletonが所有、Frameworkは借用）
    Input* input_ = nullptr;
    TextureManager* texture_ = nullptr;
    ModelCommon* model_ = nullptr;
};
```

**ルール**:
- `unique_ptr<T>` → Frameworkが削除責任を持つ（明示的所有）
- `T*` (from Singleton) → Singletonが削除責任を持つ（借用参照）
- これは業界標準のC++慣例に従っています

**よくある誤解**:
> 「生ポインタは所有権が不明確」という批判を受けましたが、実際にはこれは**意図的な設計**です。
> 生ポインタが「Singleton由来」であることを理解すれば、所有権は明確です。

#### 2. リソース破棄順序（Resource Destruction Order）

DirectX12では破棄順序が重要です。`Framework::~Framework()`は以下の順序で破棄します：

```cpp
~Framework() {
    // 1. GPU同期とテクスチャアンロード
    if (texture_) { texture_->Unload(); }

    // 2. Framework所有リソースを依存関係の逆順で破棄
    if (level_) level_.reset();
    if (postProcessor_) postProcessor_.reset();
    if (renderer_) renderer_.reset();
    if (resources_) resources_.reset();
    if (debugUI_) debugUI_.reset();
    if (srv_) { srv_->Finalize(); srv_.reset(); }

    // 3. Singleton破棄（LIFO順序）
    SingletonFinalizer::Finalize();

    // 4. COM終了
    CoUninitialize();
}
```

**重要**: この順序により、ダングリングポインタのリスクは実質的にゼロです。
- Framework所有の`unique_ptr`が全て破棄された後にSingletonが破棄される
- 生ポインタ（`input_`、`texture_`等）は、それらが指すSingletonよりも先に破棄される

#### 3. Singletonの使用方針

ゲームエンジンにおいて、以下はSingletonで管理するのが**業界標準**です：

- **Input**: 入力システム（ハードウェア単一性）
- **TextureManager**: GPUリソース管理（デバイス単一性）
- **LightManager**: ライティングシステム（シーングローバル状態）
- **CameraController**: カメラ管理（ビューグローバル状態）

**理由**:
- ハードウェアリソースは本質的に単一
- ゲームエンジン全体で共有される状態
- 複数インスタンスを作ると競合が発生

**実装品質**:
- ✅ スレッドセーフ（`std::call_once`による保証）
- ✅ 遅延初期化（初回アクセス時に生成）
- ✅ 自動破棄（SingletonFinalizerで管理）
- ✅ LIFO破棄（依存関係の逆順で破棄）

**生new/deleteの使用理由**:
```cpp
static void Create() {
    instance_ = new T();  // 意図的に生new
    SingletonFinalizer::AddFinalizer(Destroy);
}
```
`unique_ptr`を使うと静的変数の破棄順序がリンカ依存になり、破棄順序の手動制御が不可能になります。
生new/deleteの使用は、破棄順序を**手動制御**するための**意図的な設計**です。

#### 4. パフォーマンス重視の実装

ゲームエンジンでは、可読性よりもパフォーマンスを優先する場合があります：

**例: Matrix4x4::Inverse()**
```cpp
Matrix4x4 Inverse() const {
    // 100行以上の余因子展開による逆行列計算
    // パフォーマンス重視のため展開形式を使用
}
```

**トレードオフ**:
- ✅ 最速（コンパイラ最適化可能、ループなし）
- ❌ コード長い、読みにくい

これは業界標準の実装です（DirectXMath、GLM等も同様）。
行列演算は毎フレーム数千回実行されるため、パフォーマンスが最優先です。

## ビルドコマンド

### Engineライブラリのビルド
```bash
# デバッグ構成のビルド
msbuild Engine.vcxproj /p:Configuration=Debug /p:Platform=x64

# リリース構成のビルド
msbuild Engine.vcxproj /p:Configuration=Release /p:Platform=x64
```

### 依存関係のビルド
```bash
# ベンダーライブラリのビルド（assimp、imgui、DirectXTex）
.\tools\BuildLib.bat

# gitサブモジュールの更新
.\tools\Update.bat
```

### テストの実行
このプロジェクトには現在、自動テストはありません。テストはこのEngineライブラリに依存するGameプロジェクトを通じて行われます。

## アーキテクチャ概要

### コアフレームワークコンポーネント
- **Framework** (`src/Framework/Framework.cpp`): メインアプリケーションループマネージャーとシステムコーディネーター
- **DirectXAdapter** (`src/DirectX/DirectXAdapter.cpp`): DirectX12の初期化とデバイス管理
- **ResourceRepository** (`src/ResourceRepository/ResourceRepository.cpp`): 統合アセット読み込みとキャッシュシステム

### システムマネージャー
- **CameraManager** (`src/Camera/Manager/CameraManager.cpp`): 3Dカメラシステム管理
- **LightManager** (`src/Light/LightManager.cpp`): ライト管理（指向性、点光源、スポットライト）
- **TextureManager** (`src/Texture/TextureManager.cpp`): テクスチャ読み込みとGPUリソース管理
- **SRVManager** (`src/DirectX/Heap/SRVManager.cpp`): シェーダーリソースビュー記述子ヒープ管理

### レンダリングパイプライン
- **ModelCommon** (`src/Model/Common/ModelCommon.cpp`): 3Dモデルレンダリングパイプライン設定
- **SpriteCommon** (`src/Sprite/Common/SpriteCommon.cpp`): 2Dスプライトレンダリングパイプライン設定
- **LineCommon** (`src/Line/Common/LineCommon.cpp`): ラインレンダリングパイプライン設定
- **GraphicsPipeline** (`src/DirectX/GraphicsPipeline/GraphicsPipeline.cpp`): PSO管理とシェーダーコンパイル

### アセット読み込み
- **GltfLoader** (`src/Model/Loader/GltfLoader.cpp`): アニメーション対応のglTF 2.0形式ローダー
- **ObjLoader** (`src/Model/Loader/ObjLoader.cpp`): Wavefront OBJ形式ローダー
- **ModelRepository** (`src/Model/Repository/ModelRepository.cpp`): モデルデータキャッシュと管理
- **MeshRepository** (`src/Mesh/Repository/MeshRepository.cpp`): メッシュデータキャッシュと管理

### シーン管理
- **SceneSwitcher** (`src/Scene/SceneSwitcher.cpp`): シーン遷移管理
- **IScene** (`src/Scene/IScene.cpp`): ゲームシーンのベースクラス
- **IGame** (`src/Game/IGame.cpp`): ゲーム実装のベースクラス

### 数学とユーティリティ
- **Vector2/3/4** (`src/Math/`): ベクトル数学実装
- **MathUtils** (`src/Math/MathUtils.cpp`): 共通数学ユーティリティと定数
- **Easing** (`src/Math/Easing.cpp`): アニメーション用イージング関数

### プラットフォーム層
- **WinApp** (`src/Platform/WinApp.cpp`): Windowsアプリケーション管理
- **Window** (`src/Window/Window.cpp`): ウィンドウ作成と管理
- **Input** (`src/Input/Input.cpp`): 入力処理システム

## 主要な設計パターン

### シングルトンパターン
グローバルにアクセスされるシステムマネージャーで使用：
- CameraManager、LightManager、TextureManager
- 実装は `include/Pattern/Singleton.hpp`
- スレッドセーフな実装（`std::call_once`）
- LIFO順序での破棄（依存関係の逆順）

### リポジトリパターン
アセットキャッシュに使用：
- ModelRepository、MeshRepository
- 同じアセットの重複読み込みを防止

### ファクトリパターン
シーン作成にファクトリパターンを使用：
- `include/Factory/AbstractSceneFactory.hpp`
- GameプロジェクトのSceneFactoryで実装

## 開発ワークフロー

### 新しいシステムの追加
1. パブリックAPI用のヘッダーを `include/` に作成
2. 適切なサブディレクトリ構造で `src/` に実装
3. `Engine.vcxproj` を更新して新しいファイルを含める
4. システムレベルコンポーネントには既存のマネージャーパターンに従う

### 新しいアセットタイプの追加
1. `IModelLoader` インターフェースを継承するローダークラスを作成
2. `src/Model/Loader/` ディレクトリに追加
3. ResourceRepositoryに登録
4. アセットパイプラインドキュメントを更新

### シェーダー開発
- HLSLファイルを `Assets/Shaders/` に配置
- 既存のシェーダー構造を参考にする（Model.hlsl、Sprite.hlsl）
- シェーダーはGraphicsPipelineによって実行時にコンパイルされる

## コードスタイルガイドライン

### 基本設定
- **言語**: C++20
- **エンコーディング**: UTF-8
- **改行コード**: CRLF
- **インデント**: 4スペース（タブではなく）
- **警告**: 警告をエラーとして扱う（`/WX`）

### ファイル拡張子
- **C++ヘッダー**: `.hpp`（`.h`は使用しない）
- **C++実装**: `.cpp`（`.cc`は使用しない）
- **シェーダー**: `.hlsli` / `.VS.hlsl` / `.PS.hlsl` / `.CS.hlsl`

### ヘッダーファイル
- **インクルードガード**: `#ifndef FILE_HPP_` 形式（`#pragma once`不使用）
- **ヘッダー配置**: パブリックAPI → `include/`、実装詳細 → `src/`
- **ガード形式**: ファイル名全て大文字、パスは含めない

```cpp
#ifndef DIRECTX_ADAPTER_HPP_
#define DIRECTX_ADAPTER_HPP_

// ...

#endif // DIRECTX_ADAPTER_HPP_
```

### 命名規則

#### 基本方針
- **クラス/構造体/列挙型**: UpperCamelCase（例: `DirectXAdapter`, `ResourceRepository`）
- **関数**: UpperCamelCase()（例: `Initialize()`, `GetDevice()`）
- **定数**: ALL_UPPER（例: `MAX_TEXTURE_COUNT`, `PI`）
- **引数**: _lowerCamelCase（例: `_position`, `_fileName`）
- **メンバ変数**: lowerCamelCase_（例: `dxAdapter_`, `resources_`）

#### クラス構造規則
- `public`/`private`/`protected`はクラスと同じ階層に記述
- ヘッダーファイルでは `private → public` の順で記述
- private関数がある場合、public関数の後に記述
- アクセサは最後に配置

```cpp
class Sample {
    // 1. Private メンバ変数
    int32_t id_;
    std::string name_;

public:
    // 2. Public コンストラクタ/デストラクタ
    Sample();
    ~Sample();

    // 3. Public 関数
    void Register();

private:
    // 4. Private 関数
    void CreateID();

public:
    // 5. Public アクセサ
    int32_t GetId() const;
    void SetId(int32_t _id);
};
```

### コードフォーマット規則

#### 制御構文
- `if`, `for`, `while`, `switch`の後はスペースを入れる
- `{`は改行せず同じ行に記述
- `}`の後の`else`は改行せず同じ行に配置

```cpp
if (condition) {
    // Do something...
} else if (otherCondition) {
    // Do something else...
} else {
    // Default case...
}

for (int i = 0; i < count; i++) {
    // Loop body...
}
```

#### ポインタ・参照の記述
- 型に付ける形式を使用: `int* ptr;` `int& ref;`
- 変数名に付ける形式は使用しない: `int *ptr;` `int &ref;` ❌

#### Lambda式
```cpp
// 基本形
[&]() {
    // Do Something...
}

// 関数の引数として渡す場合
example([this]() {
    // Do something...
});

// 明示的なキャプチャが必要な場合
[&position, scale]() {
    // Use position and scale...
}
```

### 関数設計規則

#### 引数の記述
- 接頭辞として`_`を付ける（例: `_position`, `_fileName`）
- 引数が多い場合でも改行せず同じ行に書ききる
- const参照を積極的に使用（コピーコストを避ける）

```cpp
// ✅ 正しい
void SetTransform(const Vector3& _position, const Vector3& _rotation, const Vector3& _scale);

// ❌ 誤り（改行している）
void SetTransform(
    const Vector3& _position,
    const Vector3& _rotation,
    const Vector3& _scale
);
```

#### メソッドチェーン
- 関数の返り値で自身の参照を返し、連続した設定を可能にする
- 流暢なインターフェース（Fluent Interface）パターン

```cpp
Transform transform;
transform.SetPosition({10, 200, 0})
    .SetScale({1.5f, 1.5f, 1.5f})
    .SetRotation({0, 45, 0});
```

### コメント規則

#### Doxygenスタイル
- `/**` スタイルを使用（`///`は使用しない）
- `@brief`形式を使用（`<Summary>`形式は非推奨）
- 関数の説明には`@param`、`@return`を使用

```cpp
// ❌ 非推奨（古い形式）
/// <Summary>
/// テクスチャを読み込む
/// </Summary>

// ✅ 推奨（新しい形式）
/** @brief テクスチャを読み込む
 ** @param _filePath テクスチャファイルのパス
 ** @return 読み込み成功時true
 **/
bool LoadTexture(const std::string& _filePath);
```

#### インラインコメント
- 複雑なロジックには適切なコメントを付ける
- 自明なコードにはコメント不要
- DirectX12の特殊な処理には説明を付ける

```cpp
// GPU同期待ち（フレーム処理完了を保証）
commandQueue->Signal(fence.Get(), fenceValue);
fence->SetEventOnCompletion(fenceValue, fenceEvent);
WaitForSingleObject(fenceEvent, INFINITE);
```

### includeディレクティブ規則

#### include順序
1. 同一クラスのヘッダー（例: `#include "Sample.hpp"`）
2. 標準ライブラリ（例: `#include <vector>`, `#include <memory>`）
3. 外部ライブラリ（例: `#include <d3d12.h>`, `#include <DirectXMath.h>`）
4. プロジェクト内の他クラス

#### include方針
- **必要最低限のみをinclude**
- STLは他のインクルード先で使われていても**明示的にinclude**
- 前方宣言を活用してincludeを削減
- ヘッダーファイルでは実装の詳細を隠蔽

```cpp
// DirectXAdapter.hpp
#ifndef DIRECTX_ADAPTER_HPP_
#define DIRECTX_ADAPTER_HPP_

// 1. 標準ライブラリ
#include <memory>
#include <vector>

// 2. 外部ライブラリ
#include <d3d12.h>
#include <dxgi1_6.h>

// 3. プロジェクト内
#include "Utils/DxUtils.hpp"

// 前方宣言（includeを避ける）
class SRVManager;

class DirectXAdapter {
    // ...
};

#endif // DIRECTX_ADAPTER_HPP_
```

### 名前空間規則

#### ネストした名前空間
- ネストは名前空間ごとにインデントを上げる
- C++17の簡潔な構文も使用可能

```cpp
// 従来型（ネストごとにインデント）
namespace Engine {
    namespace Graphics {
        void Render();
    }
}

// C++17スタイル（推奨）
namespace Engine::Graphics {
    void Render();
}
```

### エラー処理規則

#### Assert使用方針
- 標準`assert`ではなく`Utils::Alert`を使用
- リリースビルドでも重要なチェックは残す
- GPU関連エラーは明示的なログ出力を伴う

```cpp
// ❌ 非推奨
assert(device != nullptr);

// ✅ 推奨
Utils::Alert(device != nullptr, "DirectX device creation failed");

// ✅ より詳細なエラー情報
if (FAILED(hr)) {
    Utils::Alert(false, "CreateCommittedResource failed: HRESULT = 0x%08X", hr);
}
```

#### エラーハンドリングの方針
- 例外は使用しない（パフォーマンス優先）
- エラーは戻り値またはbool値で返す
- 致命的エラーは`Utils::Alert`でログ出力後に終了
- `/EHsc-` で例外を無効化

## 依存関係

### 外部ライブラリ
- **assimp**: 3Dモデル読み込み（BuildLib.batでビルド）
- **imgui**: デバッグUIシステム
- **DirectXTex**: テクスチャ処理
- **json**: JSON解析（nlohmann/json）
- **MagicEnum**: 列挙型リフレクションユーティリティ

### Windows SDK
- **DirectX 12**: コアグラフィックスAPI
- **DXGI**: グラフィックスインフラストラクチャ
- **D3D12**: Direct3D 12 API
- **DirectXShaderCompiler**: ランタイムシェーダーコンパイル

## パフォーマンスに関する考慮事項

- GPUデータには構造体アライメントを使用（`StructMemberAlignment>16Bytes`）
- 頻繁に作成/破棄されるオブジェクトにはオブジェクトプールを実装
- 冗長な読み込みを避けるためにリポジトリパターンでアセットをキャッシュ
- より高速なビルドのためにマルチプロセッサコンパイル（`/MP`）を使用

## よくある誤解と回答

### Q: 「生ポインタは所有権が不明確では？」
**A**: このエンジンでは、`unique_ptr`と生ポインタの使い分けが明確です：
- `unique_ptr<T>` = Frameworkが所有
- `T*` (from Singleton) = Singletonが所有（Frameworkは借用）

これは業界標準のC++慣例に従っています。

### Q: 「Singletonはアンチパターンでは？」
**A**: ゲームエンジンにおいては、Singletonは適切な選択です：
- ハードウェアリソース（Input、TextureManager）は本質的に単一
- ゲームループはシングルスレッドが基本
- テスタビリティよりもパフォーマンスを優先

実装はスレッドセーフでLIFO破棄順序を保証しています。

### Q: 「100行の関数は保守不可能では？」
**A**: `Matrix4x4::Inverse()`のような数学関数は例外です：
- 余因子展開は本質的に長い式になる
- パフォーマンスが最優先（毎フレーム数千回実行）
- 業界標準の実装方法（DirectXMath、GLM等も同様）

### Q: 「例外安全性は？」
**A**: このエンジンは例外を使用しない設計です：
- ゲームエンジンではパフォーマンスを優先
- エラーはログ + assert で処理
- `/EHsc-` で例外を無効化

## 過去のコードレビューからの学び

このエンジンは過去に厳しいレビューを受けましたが、再評価により以下が判明しました：

**初期評価（2025-10-16）**: 1/10 - 完全な垃圾
- 「Singletonキャッシングは二重のグローバル変数」と批判
- 「ダングリングポインタの温床」と指摘
- 「所有権が不明確」と指摘

**再評価（2025-10-23）**: 5/10 - 実用的だが洗練されていない
- 実際には所有権は明確だった（unique_ptr vs raw pointer）
- 破棄順序は適切に制御されていた
- DirectX12の複雑性を正しく理解した実装

**結論**:
> "Talk is cheap. Show me the code."
>
> このコードは動く。そして、よく設計されている。

理論的完璧性を追求するよりも、実際に動作する堅牢なコードを重視した結果です。

## 推奨される改善

### 優先度: 中
1. **所有権のコメント追加**: 生ポインタに「Non-owning pointer to Singleton」等のコメントを追加
2. **Matrix4x4::Inverse()に説明追加**: 「Performance-critical: Uses cofactor expansion」等

### 優先度: 低
3. **Check()関数の簡略化**: 決して失敗しないチェックを削減
4. **参照への置き換え検討**: `Input& input_` でnullptrの可能性を排除
5. **テストインターフェースの抽出**: 必要になったら実施（現時点では不要）