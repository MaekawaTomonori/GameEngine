# リファクタリング ToDoリスト

**最終更新**: 2025-10-02
**プロジェクト**: PortfolioGame Engine
**ベースドキュメント**: HackList.md + コード分析レポート

このドキュメントは、優先順位と実装順序を明確化したリファクタリングタスクリストです。

---

## 📋 進捗サマリー

| フェーズ | タスク数 | 完了 | 進行中 | 未着手 | 進捗率 |
|---------|---------|------|--------|--------|--------|
| Phase 1 (緊急) | 3 | 0 | 0 | 3 | 0% |
| Phase 2 (重要) | 7 | 0 | 0 | 7 | 0% |
| Phase 3 (改善) | 6 | 0 | 0 | 6 | 0% |
| Phase 4 (最適化) | 3 | 0 | 0 | 3 | 0% |
| **合計** | **19** | **0** | **0** | **19** | **0%** |

---

## 🔴 Phase 1: 緊急対応（1-2週間）

これらのタスクは機能的に重要で、システムの正確性に影響します。

### ✅ P1-1: PostProcessExecutor SRV実装完了

**優先度**: 🔴 Critical
**見積工数**: 2-3日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象ファイル**:
- `src/PostProcess/Executor/PostProcessExecutor.cpp` (行122, 166, 178-179)
- `src/PostProcess/Executor/PostProcessExecutor.hpp`

**実装内容**:
1. [ ] SRVManagerとの統合実装
   - `sceneSRV_`メンバの適切な初期化
   - SRVManager経由でのSRV作成
2. [ ] CopyImgシェーダーのPSO実装
   - PipelineStateObjectの完全設定
   - RootSignatureの実装
3. [ ] Draw()メソッドでのSRV設定
   - シーンテクスチャのSRV適用
   - ポストエフェクトチェーンの検証
4. [ ] 統合テストと動作確認
   - Grayscale, Vignette, BoxBlur の動作検証

**検証方法**:
```cpp
// テストコード例
PostProcessExecutor executor;
executor.Initialize(adapter, srv, debug);
executor.Add(std::make_unique<Grayscale>(...), "Grayscale");
// シーンレンダリング後のエフェクト適用を確認
```

**完了条件**:
- [ ] すべてのポストエフェクトが正しく適用される
- [ ] SRVManagerとの統合が完了
- [ ] メモリリークがない

---

### ✅ P1-2: リソースリークチェックのビルド条件分岐

**優先度**: 🔴 Critical
**見積工数**: 30分
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象ファイル**:
- `src/DirectX/DirectXAdapter.cpp` (行67)

**実装内容**:
```cpp
// 現状（行67）
D3DResourceLeakChecker _lc;

// 修正後
#ifdef _DEBUG
    D3DResourceLeakChecker _lc;
#endif
```

**完了条件**:
- [ ] Releaseビルドでリークチェッカーが無効化される
- [ ] Debugビルドで正常に動作する

---

### ✅ P1-3: assert → 例外処理への移行（Critical箇所）

**優先度**: 🔴 Critical
**見積工数**: 1-2日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象ファイル**:
- `src/DirectX/DirectXAdapter.cpp`
- `src/Texture/TextureManager.cpp`
- その他assertを使用する29箇所

**実装内容**:
```cpp
// 修正前
assert(resource != nullptr);

// 修正後
if (!resource) {
    Log::Send(Log::Level::ERR, "Resource creation failed");
    throw std::runtime_error("Critical resource allocation failure");
}
```

**段階的実装**:
1. [ ] DirectXAdapterの重要なassertを例外に変更
2. [ ] TextureManagerのassertを例外に変更
3. [ ] その他のクリティカルなassertを例外に変更
4. [ ] エラーハンドリングのテスト

**完了条件**:
- [ ] Releaseビルドでもエラー検出可能
- [ ] 適切なログ出力
- [ ] 例外安全性の確保

---

## 🟡 Phase 2: 重要改善（3-4週間）

設計品質と保守性を向上させるタスク群です。

### ✅ P2-1: EngineConfig統合システム構築

**優先度**: 🟡 High
**見積工数**: 3-4日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象ファイル**:
- `src/Config/Config.hpp` (新規: EngineConfigクラス追加)
- `src/Config/Config.cpp`
- `Assets/Config/EngineSettings.json` (新規作成)

**実装内容**:
1. [ ] EngineConfigクラス設計
```cpp
// Config.hpp に追加
namespace GameEngine {
    struct RenderingLimits {
        uint32_t maxLines = 1000;
        struct LightLimits {
            uint32_t directional = 20;
            uint32_t point = 20;
            uint32_t spot = 20;
        } lights;
        Vector2 defaultSpriteSize {100.0f, 100.0f};
    };

    struct CameraDefaults {
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        float fov = 45.0f;
    };

    struct SpriteDefaults {
        float projectionNear = 0.0f;
        float projectionFar = 100.0f;
    };

    class EngineConfig {
        RenderingLimits rendering_;
        CameraDefaults camera_;
        SpriteDefaults sprite_;

    public:
        static EngineConfig& GetInstance();
        void LoadFromFile(const std::string& path);

        const RenderingLimits& GetRenderingLimits() const;
        const CameraDefaults& GetCameraDefaults() const;
        const SpriteDefaults& GetSpriteDefaults() const;
    };
}
```

2. [ ] JSON設定ファイル作成
```json
// Assets/Config/EngineSettings.json
{
  "rendering": {
    "maxLines": 1000,
    "lights": {
      "directional": 20,
      "point": 20,
      "spot": 20
    },
    "defaultSpriteSize": [100.0, 100.0]
  },
  "camera": {
    "nearPlane": 0.1,
    "farPlane": 100.0,
    "fov": 45.0
  },
  "sprite": {
    "projectionNear": 0.0,
    "projectionFar": 100.0
  }
}
```

3. [ ] JSON読み込み実装
4. [ ] Framework初期化での読み込み

**完了条件**:
- [ ] JSON設定ファイルから値が読み込まれる
- [ ] デフォルト値のフォールバック機能
- [ ] 実行時の設定変更サポート

---

### ✅ P2-2: Line.hpp のMAX_LINES設定化

**優先度**: 🟡 High
**見積工数**: 1日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**依存**: P2-1完了後

**対象ファイル**:
- `include/Line.hpp` (行39)
- `src/Line/Line.cpp`
- `src/Line/Common/LineCommon.cpp`

**実装内容**:
```cpp
// Line.hpp 修正前
const uint32_t MAX_LINES = 1000;

// Line.hpp 修正後
class Line {
    uint32_t maxLines_;  // インスタンス変数化
public:
    explicit Line(uint32_t maxLines = 0);  // 0 = 設定ファイルから取得
};

// Line.cpp
Line::Line(uint32_t maxLines)
    : maxLines_(maxLines == 0
        ? EngineConfig::GetInstance().GetRenderingLimits().maxLines
        : maxLines) {
    // バッファ確保
}
```

**完了条件**:
- [ ] 設定ファイルから最大ライン数を読み込み
- [ ] コンストラクタでの明示的指定も可能
- [ ] 既存コードの動作を保証

---

### ✅ P2-3: LightManager ライト数制限の設定化

**優先度**: 🟡 High
**見積工数**: 1日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**依存**: P2-1完了後

**対象ファイル**:
- `src/Light/LightManager.hpp` (行38)
- `src/Light/LightManager.cpp`

**実装内容**:
```cpp
// LightManager.hpp 修正前
const LightCount MAX_COUNT {20, 20, 20};

// LightManager.hpp 修正後
class LightManager {
    LightCount maxCount_;  // インスタンス変数化
public:
    void Initialize(DirectXAdapter* adapter, DebugUI* debug,
                    const LightCount& maxCount = {0, 0, 0});
};

// LightManager.cpp
void LightManager::Initialize(..., const LightCount& maxCount) {
    const auto& config = EngineConfig::GetInstance().GetRenderingLimits().lights;
    maxCount_ = {
        maxCount.directional == 0 ? config.directional : maxCount.directional,
        maxCount.point == 0 ? config.point : maxCount.point,
        maxCount.spot == 0 ? config.spot : maxCount.spot
    };
}
```

**完了条件**:
- [ ] 設定ファイルからライト数制限を読み込み
- [ ] GPU性能に応じた動的調整の基盤
- [ ] 既存のライト処理が正常動作

---

### ✅ P2-4: Sprite マジックナンバー削除

**優先度**: 🟡 High
**見積工数**: 1日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**依存**: P2-1完了後

**対象ファイル**:
- `src/Sprite/Sprite.cpp` (行73, 117)
- `src/Sprite/Common/SpriteCommon.hpp`

**実装内容**:
```cpp
// Sprite.cpp 行73 修正前
size_ = {100, 100};

// Sprite.cpp 修正後
const auto& config = EngineConfig::GetInstance().GetSpriteDefaults();
size_ = config.defaultSpriteSize;

// Sprite.cpp 行117 修正前
Matrix::OrthographicLH(0, 100.f, ...);

// Sprite.cpp 修正後
const auto& config = EngineConfig::GetInstance().GetSpriteDefaults();
Matrix::OrthographicLH(config.projectionNear, config.projectionFar, ...);
```

**完了条件**:
- [ ] すべてのマジックナンバーを設定化
- [ ] Spriteのデフォルト動作を保証
- [ ] 設定変更でのスケール対応

---

### ✅ P2-5: Camera デフォルト値の設定化

**優先度**: 🟡 High
**見積工数**: 1日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**依存**: P2-1完了後

**対象ファイル**:
- `src/Camera/Camera.hpp` (行29)
- `src/Camera/Camera.cpp` (行34-35)

**実装内容**:
```cpp
// Camera.hpp 修正前
float far_ = 100.0f;

// Camera.hpp 修正後
float far_;  // コンストラクタで初期化

// Camera.cpp コンストラクタ
Camera::Camera() {
    const auto& config = EngineConfig::GetInstance().GetCameraDefaults();
    near_ = config.nearPlane;
    far_ = config.farPlane;
    fov_ = config.fov;
}
```

**完了条件**:
- [ ] カメラパラメータが設定ファイルから読み込まれる
- [ ] デバッグUI範囲の動的計算
- [ ] シーン規模に応じた調整基盤

---

### ✅ P2-6: ヘッダー拡張子の統一 (.h → .hpp)

**優先度**: 🟡 Medium
**見積工数**: 2-3時間
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象ファイル**:
- `src/DirectX/Heap/SRVManager.h` → `SRVManager.hpp`
- `src/Light/DirectionalLight/DirectionalLight.h` → `DirectionalLight.hpp`
- `src/Light/PointLight/PointLight.h` → `PointLight.hpp`
- `src/Light/SpotLight/SpotLight.h` → `SpotLight.hpp`
- `src/Light/RawLight.h` → `RawLight.hpp`

**実装内容**:
1. [ ] ファイル名変更
2. [ ] インクルードガード更新
3. [ ] すべての #include ディレクティブ更新
4. [ ] vcxproj ファイル更新
5. [ ] ビルド検証

**自動化スクリプト案**:
```powershell
# PowerShell スクリプト
$files = @(
    "src/DirectX/Heap/SRVManager",
    "src/Light/DirectionalLight/DirectionalLight",
    # ...
)
foreach ($file in $files) {
    git mv "$file.h" "$file.hpp"
    # 全ファイルで #include 更新
    (Get-Content -Path **/*.cpp, **/*.hpp) |
        ForEach-Object { $_ -replace "$file\.h", "$file.hpp" } |
        Set-Content
}
```

**完了条件**:
- [ ] すべての.hファイルが.hppに変更
- [ ] ビルドエラーなし
- [ ] コミット履歴の保持（git mv使用）

---

### ✅ P2-7: DRY原則違反の解消

**優先度**: 🟡 Medium
**見積工数**: 2-3日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象箇所**:
- 各Commonクラスの初期化パターン（SpriteCommon, ModelCommon, LineCommon, SkyCommon）
- デバッグUI登録コードの重複

**実装内容**:
1. [ ] CommonBase基底クラスの作成
```cpp
// src/Common/RenderingCommon.hpp (新規)
class RenderingCommon {
protected:
    DirectXAdapter* adapter_ = nullptr;
    DebugUI* debugUI_ = nullptr;

    void Setup(DirectXAdapter* adapter, DebugUI* debug);
    virtual void InitializePipeline() = 0;

public:
    virtual ~RenderingCommon() = default;
};
```

2. [ ] 各Commonクラスのリファクタリング
```cpp
class SpriteCommon : public RenderingCommon {
public:
    void Initialize(DirectXAdapter* adapter, DebugUI* debug) override {
        Setup(adapter, debug);
        InitializePipeline();
    }
protected:
    void InitializePipeline() override {
        // Sprite固有のパイプライン設定
    }
};
```

3. [ ] デバッグUI登録ヘルパー作成
```cpp
// DebugUI.hpp に追加
template<typename T>
void RegisterPropertySlider(const char* label, T& value, T min, T max) {
    RegisterCommand(label, [&]() {
        ImGui::SliderScalar(label, ImGuiDataType_Float, &value, &min, &max);
    });
}
```

**完了条件**:
- [ ] 共通初期化パターンの基底クラス化
- [ ] コード重複の30%以上削減
- [ ] 既存機能の動作保証

---

## 🟢 Phase 3: アーキテクチャ改善（1-2ヶ月）

長期的な設計品質向上のためのタスク群です。

### ✅ P3-1: ServiceLocatorパターン実装

**優先度**: 🟢 Medium
**見積工数**: 5-7日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象ファイル**:
- `src/System/ServiceLocator.hpp` (新規)
- `src/System/ServiceLocator.cpp` (新規)

**実装内容**:
```cpp
// ServiceLocator.hpp
class ServiceLocator {
    std::unordered_map<std::type_index, void*> services_;

public:
    template<typename T>
    void Register(T* service) {
        services_[std::type_index(typeid(T))] = service;
    }

    template<typename T>
    T* Get() const {
        auto it = services_.find(std::type_index(typeid(T)));
        if (it != services_.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }

    static ServiceLocator& GetInstance();
};
```

**段階的実装**:
1. [ ] ServiceLocatorクラスの実装
2. [ ] Framework初期化での登録
3. [ ] Singleton使用箇所の特定（15+箇所）
4. [ ] 優先度高い箇所から段階的移行

**完了条件**:
- [ ] ServiceLocatorが正常動作
- [ ] 少なくとも5つのSingletonを移行
- [ ] ユニットテスト作成

---

### ✅ P3-2: Singleton削減 - Input/TextureManager

**優先度**: 🟢 Medium
**見積工数**: 2-3日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**依存**: P3-1完了後

**対象クラス**:
- `Input`
- `TextureManager`

**実装内容**:
```cpp
// Framework.hpp 修正
class Framework {
    // Singleton削除
    // Input* input_ = nullptr;  // 削除
    // TextureManager* texture_ = nullptr;  // 削除

    // 直接所有に変更
    std::unique_ptr<Input> input_;
    std::unique_ptr<TextureManager> texture_;

public:
    Input* GetInput() const { return input_.get(); }
    TextureManager* GetTextureManager() const { return texture_.get(); }
};

// Framework.cpp 初期化
void Framework::Initialize() {
    input_ = std::make_unique<Input>();
    input_->Initialize(windows_->GetWindowHandle(), windows_->GetInstanceHandle());

    texture_ = std::make_unique<TextureManager>();
    texture_->Initialize(dxAdapter_.get(), srv_.get());

    // ServiceLocatorに登録
    ServiceLocator::GetInstance().Register(input_.get());
    ServiceLocator::GetInstance().Register(texture_.get());
}
```

**移行手順**:
1. [ ] Input/TextureManagerのSingleton削除
2. [ ] Framework直接所有への変更
3. [ ] ServiceLocator登録
4. [ ] 全使用箇所の更新
5. [ ] 動作検証

**完了条件**:
- [ ] SingletonFinalizerからInput/TextureManager削除
- [ ] すべての参照がServiceLocator経由
- [ ] メモリリークなし

---

### ✅ P3-3: Singleton削減 - CameraManager系

**優先度**: 🟢 Medium
**見積工数**: 2-3日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**依存**: P3-2完了後

**対象クラス**:
- `CameraController`
- `CameraDirector`

**実装内容**:
同様のパターンでFramework直接所有に変更

**完了条件**:
- [ ] カメラ関連Singletonの削除
- [ ] ServiceLocator経由のアクセス
- [ ] 既存機能の動作保証

---

### ✅ P3-4: Singleton削減 - Rendering系

**優先度**: 🟢 Medium
**見積工数**: 3-4日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**依存**: P3-3完了後

**対象クラス**:
- `SpriteCommon`
- `ModelCommon`
- `LineCommon`
- `SkyCommon`

**実装内容**:
Rendering系Commonクラスの所有権整理

**完了条件**:
- [ ] Rendering系Singletonの削減
- [ ] 依存性注入パターンの確立
- [ ] パフォーマンス影響の確認

---

### ✅ P3-5: Singleton削減 - LightManager

**優先度**: 🟢 Low
**見積工数**: 1-2日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**依存**: P3-4完了後

**対象クラス**:
- `LightManager`

**完了条件**:
- [ ] LightManager Singletonの削除
- [ ] Singleton使用を最小限に抑制（目標: 5個以下）

---

### ✅ P3-6: TextureManager ResourceBarrier統合

**優先度**: 🟢 Low
**見積工数**: 2-3日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象ファイル**:
- `src/Texture/TextureManager.cpp` (行89-96)
- `src/DirectX/Resource/DX12Resource.hpp`

**実装内容**:
1. [ ] DX12Resourceラッパーの拡張
```cpp
// DX12Resource.hpp に追加
class DX12Resource {
public:
    void TransitionBarrier(
        ID3D12GraphicsCommandList* cmdList,
        D3D12_RESOURCE_STATES before,
        D3D12_RESOURCE_STATES after
    );
};
```

2. [ ] TextureManager統合
```cpp
// TextureManager.cpp 行89-96 修正
// 修正前: 手動でD3D12_RESOURCE_BARRIER設定

// 修正後
wrapper->TransitionBarrier(
    cmdList,
    D3D12_RESOURCE_STATE_COPY_DEST,
    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
);
```

**完了条件**:
- [ ] すべてのResourceBarrierがラッパー経由
- [ ] DirectXTexとの整合性確保
- [ ] テクスチャ読み込みの正常動作

---

## ⚡ Phase 4: パフォーマンス最適化（2-3ヶ月）

実行時パフォーマンス向上のためのタスク群です。

### ✅ P4-1: オブジェクトプールパターン実装

**優先度**: ⚡ Optimization
**見積工数**: 5-7日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象クラス**:
- `Sprite`
- `Line`
- `Mesh` (頻繁に生成される場合)

**実装内容**:
```cpp
// src/System/ObjectPool.hpp (新規)
template<typename T>
class ObjectPool {
    std::vector<std::unique_ptr<T>> pool_;
    std::vector<T*> available_;
    std::vector<T*> inUse_;
    size_t maxSize_;

public:
    explicit ObjectPool(size_t maxSize = 100) : maxSize_(maxSize) {
        pool_.reserve(maxSize);
    }

    T* Acquire() {
        if (available_.empty()) {
            if (pool_.size() < maxSize_) {
                pool_.emplace_back(std::make_unique<T>());
                available_.push_back(pool_.back().get());
            } else {
                return nullptr;  // プール枯渇
            }
        }

        T* obj = available_.back();
        available_.pop_back();
        inUse_.push_back(obj);
        return obj;
    }

    void Release(T* obj) {
        auto it = std::find(inUse_.begin(), inUse_.end(), obj);
        if (it != inUse_.end()) {
            inUse_.erase(it);
            available_.push_back(obj);
            obj->Reset();  // 再利用のための初期化
        }
    }

    void Clear() {
        inUse_.clear();
        available_.clear();
        for (auto& obj : pool_) {
            available_.push_back(obj.get());
        }
    }
};
```

**段階的実装**:
1. [ ] ObjectPoolクラステンプレート実装
2. [ ] Spriteプール実装とベンチマーク
3. [ ] Lineプール実装とベンチマーク
4. [ ] パフォーマンス測定と最適化

**完了条件**:
- [ ] 頻繁な生成/破棄でのパフォーマンス向上（目標: 20%以上）
- [ ] メモリ使用量の安定化
- [ ] プール枯渇時のフォールバック実装

---

### ✅ P4-2: 非同期アセット読み込みシステム

**優先度**: ⚡ Optimization
**見積工数**: 7-10日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手

**対象ファイル**:
- `src/ResourceRepository/ResourceRepository.hpp`
- `src/Model/Repository/ModelRepository.cpp`
- `src/Texture/TextureManager.cpp`

**実装内容**:
```cpp
// ResourceRepository.hpp に追加
class AsyncLoader {
    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;

public:
    AsyncLoader(size_t numThreads = 2);
    ~AsyncLoader();

    template<typename Func>
    std::future<void> Enqueue(Func&& func);

    void WaitAll();
};

class ResourceRepository {
    std::unique_ptr<AsyncLoader> asyncLoader_;

public:
    std::future<Model*> LoadModelAsync(const std::string& path);
    std::future<Texture*> LoadTextureAsync(const std::string& path);
};
```

**段階的実装**:
1. [ ] AsyncLoaderスレッドプール実装
2. [ ] テクスチャ非同期読み込み実装
3. [ ] モデル非同期読み込み実装
4. [ ] ローディング画面との統合
5. [ ] スレッドセーフティ検証

**完了条件**:
- [ ] メインスレッドのブロッキングなし
- [ ] ローディング時間の短縮（目標: 30%以上）
- [ ] スレッドセーフなリソース管理
- [ ] デッドロック・競合状態の検証

---

### ✅ P4-3: マルチスレッドレンダリングコマンド生成

**優先度**: ⚡ Optimization
**見積工数**: 10-14日
**担当者**: _未割当_
**ステータス**: ⬜ 未着手
**前提条件**: DirectX12マルチスレッド対応の理解

**対象ファイル**:
- `src/Renderer/Renderer.hpp`
- `src/Renderer/Renderer.cpp`

**実装内容**:
```cpp
// Renderer.hpp
class Renderer {
    struct CommandBatch {
        std::vector<std::function<void(ID3D12GraphicsCommandList*)>> commands;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
    };

    std::vector<CommandBatch> batches_;
    size_t numThreads_ = 4;

public:
    void RecordCommandsParallel();
    void ExecuteCommandLists();
};
```

**段階的実装**:
1. [ ] コマンドバッチング機構実装
2. [ ] 複数CommandAllocator/CommandList管理
3. [ ] 並列コマンド記録実装
4. [ ] 同期機構とバリア管理
5. [ ] パフォーマンス検証

**完了条件**:
- [ ] 複雑シーンでの描画パフォーマンス向上（目標: 15%以上）
- [ ] CPUボトルネックの解消
- [ ] レンダリング正確性の保証

---

## 📊 進捗管理

### 週次チェックリスト

**毎週金曜日に更新**:
- [ ] 今週完了したタスク数
- [ ] 発生したブロッカー
- [ ] 次週の目標設定
- [ ] 技術的負債の追加/削除

### マイルストーン

| マイルストーン | 目標日 | 達成条件 |
|--------------|--------|---------|
| M1: Phase 1完了 | Week 2 | P1-1, P1-2, P1-3すべて完了 |
| M2: Phase 2完了 | Week 6 | EngineConfig統合、ハードコード削除 |
| M3: Phase 3完了 | Week 14 | Singleton 50%削減 |
| M4: Phase 4完了 | Week 22 | パフォーマンス目標達成 |

---

## 🔧 実装ガイドライン

### コミット規約

```
[Phase] <Type>: <Summary>

<詳細説明>

Task: P<Phase>-<Number>
Refs: #<Issue番号>
```

**例**:
```
[P1] feat: Implement SRV integration in PostProcessExecutor

- SRVManager統合による適切なSRV作成
- CopyImgシェーダーのPSO実装完了
- 統合テスト追加

Task: P1-1
Refs: #42
```

### ブランチ戦略

- `main`: 安定版
- `develop`: 開発統合ブランチ
- `refactor/p1-1-postprocess-srv`: Phase 1, Task 1
- `refactor/p2-1-engine-config`: Phase 2, Task 1

### テスト戦略

各タスク完了時:
1. ユニットテスト作成/更新
2. 統合テスト実行
3. パフォーマンステスト（Phase 4のみ）
4. メモリリークチェック
5. デバッグビルド/リリースビルド両方で検証

---

## 📚 参考資料

- `design-philosophy.md`: 開発原則
- `HackList.md`: 技術的負債詳細
- `CLAUDE.md`: プロジェクト構造ガイド
- コード分析レポート（2025-10-02）

---

**次のアクション**: P1-1 PostProcessExecutor SRV実装から開始してください。
