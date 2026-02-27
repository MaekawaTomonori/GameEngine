# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## プロジェクト概要

これはC++20 DirectX12ベースのゲームエンジンです。Engineディレクトリには、レンダリング、入力処理、シーン管理、リソース読み込みなどの主要なゲームエンジン機能を提供する静的ライブラリ（.lib）が含まれています。

## ディレクトリ構造

- `include/` - 特にゲームで使うことを想定したクラスのヘッダ
- `src/` - クラス
- `externals` - 外部ライブラリ


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

## ビルド/テスト 

ビルドや起動テストはユーザーが行うためテストまで行う必要はない
編集・提案の終了をもって、ユーザーが実行確認を行うものとする

## パフォーマンスに関する考慮事項

- GPUデータには構造体アライメントを使用（`StructMemberAlignment>16Bytes`）
- 頻繁に作成/破棄されるオブジェクトにはオブジェクトプールを実装
- 冗長な読み込みを避けるためにリポジトリパターンでアセットをキャッシュ
- より高速なビルドのためにマルチプロセッサコンパイル（`/MP`）を使用

## 詳細ドキュメント
  
- 設計思想 : `@docs/design-philosophy.md`
- コードルール : `@docs/coderule.md`
