# Model 設計ドキュメント

## 設計の核心

**「操作用」と「描画用」の完全分離**

| | `Model` | `RenderModel` |
|---|---|---|
| **役割** | ゲームからの操作受け口 | Engine 内部の描画オブジェクト |
| **所有者** | Game（`unique_ptr<Model>` で保持） | `ModelCommon`（`unique_ptr` で管理） |
| **触れる人** | Game 開発者 | Engine 内部のみ |
| **Update/Draw** | 持たない | 持つ（自動呼び出し） |

---

## `Model` — 操作用ラッパ

```cpp
// Engine/include/Model.hpp

class RenderModel;  // forward declaration のみ（Game に型定義を漏らさない）

class Model {
public:
    Model() = default;
    explicit Model(const std::string& path);  // 構築 = Engine へ自動登録
    ~Model();                                  // 破棄 = Engine から自動解除 (RAII)

    // モデル差し替え
    void Load(const std::string& path);

    // --- 以下 setter のみ。戻り値 Model& でメソッドチェーン可 ---

    // Transform（Model 自身が保持）
    Model& SetTranslate(const Vector3& translate);
    Model& SetRotate(const Vector3& rotate);
    Model& SetScale(const Vector3& scale);

    // 可視制御
    Model& SetVisible(bool visible);

    // マテリアル
    Model& SetColor(const Vector4& color);
    Model& SetTexture(const std::string& path);
    Model& SetEnvironmentTexture(const std::string& path);
    Model& SetTilingMultiply(const Vector2& tiling);

    // アニメーション
    Model& SetAnimationEnable(bool enable);

private:
    Transform transform_{};
    bool visible_ = true;
    uint32_t handle_;
};
```

**制約:**
- `Update()` / `Draw()` は持たない
- `RenderModel` の型定義は Game TU に絶対に漏れない
- コピー禁止（`unique_ptr` での管理が前提）
- handle_をキーとしてインスタンシングへの項目登録など操作をする
---

## `RenderModel` — 描画用オブジェクト

```cpp
// Engine/src/Model/RenderModel.hpp（Game 非公開）

class RenderModel {
public:
    void Initialize(const std::string& path);
    void Load(const std::string& path);     // 差し替え

    // Model から呼ばれる setter
    void SetTransform(const Transform& t);
    void SetVisible(bool visible);
    void SetColor(const Vector4& color);
    void SetTexture(const std::string& path);
    void SetEnvironmentTexture(const std::string& path);
    void SetTilingMultiply(const Vector2& tiling);
    void SetAnimationEnable(bool enable);

    // ModelCommon からのみ呼ばれる
    void Update();
    void Draw();

private:
    ModelData* data_ = nullptr;
    std::unique_ptr<Mesh> mesh_;
    std::optional<Skeleton> pose_;
    SkinCluster skinCluster_;
    // GPU transform バッファ (wr_, wd_, cr_, cd_)
    // アニメーション状態
    Transform transform_{};
    bool visible_ = true;
};
```

---

## `ModelCommon` — 管理者

```cpp
// Engine/src/Model/Common/ModelCommon.hpp

class ModelCommon : public Common {
public:
    // Model コンストラクタ → 呼ばれる
    RenderModel* Register(const std::string& path);

    // Model デストラクタ → 呼ばれる
    void Unregister(RenderModel* target);

    // Framework から呼ばれる（毎フレーム）
    void Update() override;
    void Draw(Renderer* renderer) override;

private:
    std::vector<std::unique_ptr<RenderModel>> renderModels_;
    // ...（既存フィールド: pipeline, resource, srv 等）
};
```

**描画フロー:**
```
Framework::Update() → ModelCommon::Update() → 全 RenderModel::Update()
Framework::Draw()   → ModelCommon::Draw()
                         → visible_ == true のもののみ RenderModel::Draw()
```

---

## ライフサイクル

```
Model m("player")
  └─ ModelCommon::Register("player")
       ├─ ResourceRepository にキャッシュがなければ Loader を呼ぶ
       ├─ new RenderModel → Initialize("player")
       └─ renderModels_ に追加、生ポインタを返す
  └─ m.renderModel_ = 返ってきたポインタ

m.SetTranslate({1,0,0})
  └─ m.transform_.translate = {1,0,0}
  └─ m.renderModel_->SetTransform(m.transform_)

// unique_ptr<Model> が破棄されると:
~Model()
  └─ ModelCommon::Unregister(renderModel_)
       └─ renderModels_ から該当エントリを削除（unique_ptr 破棄）
```

---

## Game 側の使用例

```cpp
class Player {
    std::unique_ptr<Model> model_;

    void Initialize() {
        Model::Preload("player");          // 任意の事前キャッシュ

        model_ = std::make_unique<Model>("player");
        model_->SetTranslate({0.0f, 0.0f, 0.0f})
               .SetColor({1.0f, 1.0f, 1.0f, 1.0f});
    }

    void OnDamage() {
        model_->Load("player_damaged");    // 差し替え
    }

    void SetInvisible() {
        model_->SetVisible(false);
    }

    // Update/Draw の呼び出しは一切不要
};
```

---

## 変更ファイル一覧

| ファイル | 変更種別 | 内容 |
|---|---|---|
| `Engine/include/Model.hpp` | 変更 | 操作用 thin wrapper に書き直し |
| `Engine/src/Model/Model.cpp` | 変更 | RenderModel への委譲のみ |
| `Engine/src/Model/RenderModel.hpp` | 新規 | 現 Model 内部実装を移動 |
| `Engine/src/Model/RenderModel.cpp` | 新規 | 現 Model.cpp 実装を移動 |
| `Engine/src/Model/Common/ModelCommon.hpp` | 変更 | Register/Unregister、RenderModel リスト管理 |
| `Engine/src/Model/Common/ModelCommon.cpp` | 変更 | Update/Draw ループを RenderModel リスト駆動に変更 |
| `Engine/Engine.vcxproj` | 変更 | RenderModel.hpp/.cpp を追加 |
