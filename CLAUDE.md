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
- 実装は `src/System/Singleton/Singleton.cpp`

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

- **言語**: C++20
- **エンコーディング**: UTF-8
- **インデント**: 4スペース
- **インクルードガード**: `#ifndef FILE_HPP_` 形式
- **ヘッダー構成**: パブリックヘッダーは `include/`、プライベートは `src/`
- **警告**: 警告をエラーとして扱う（`/WX`）

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