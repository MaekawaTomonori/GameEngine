# UI システム アーキテクチャ

## 概要

UI システムは `Canvas → Layer → IComponent` の階層構造で構成される。

```
Canvas
 ├── EventSystem                         アクションキー → コールバック の登録・実行
 └── layers_: vector<Layer>              ZOrder 昇順で描画
      └── Layer
           ├── name, active, positionOffset, zOrder
           └── components_: vector<IComponent>
                ├── Element   最小単位（単一 Sprite）
                ├── Button    bg Sprite + fg Sprite（ホバー視覚フィードバック付き）
                └── (Slider / Toggle / ProgressBar … 将来追加)
```

---

## クラス設計

### IComponent — 共通インターフェース

`Engine/include/Ui/IComponent.hpp`

全ての UI コンポーネントが実装する純粋仮想インターフェース。

```cpp
class IComponent {
public:
    struct Rect { Vector2 position; Vector2 size; };

    virtual void Update(const Vector2& parentOffset) = 0;
    virtual void Draw() const = 0;
    virtual void Debug(const std::vector<std::string>& availableActions) = 0;

    virtual std::string GetName() const = 0;
    virtual std::string GetUUID() const = 0;
    virtual bool        IsVisible() const = 0;
    virtual Rect        GetRect() const = 0;

    // インタラクション（不要なら継承先でデフォルト無効のまま使用）
    virtual bool IsInteractive() const { return false; }
    virtual void SetHovered(bool hovered) {}
    virtual void SetPressed(bool pressed) {}

    // イベントバインド
    virtual bool               HasEvent(EventKey event) const { return false; }
    virtual const std::string& GetActionKey(EventKey event) const;

    // JSON シリアライズ
    virtual std::string    GetTypeName() const = 0;  // "Element" / "Button" など
    virtual nlohmann::json Serialize()   const = 0;

    virtual ~IComponent() = default;
};
```

---

### Layer — コンポーネントのグループ

`Engine/include/Ui/Layer.hpp`

Canvas が複数の Layer を ZOrder 順に持つ。各 Layer は独立して Active/Inactive を切り替えられる。

**プロパティ（JSON 保存）**

| プロパティ | 説明 |
|-----------|------|
| name | 識別用の名前 |
| active | false にすると Update / Draw / インタラクションをスキップ |
| positionOffset | Layer 内の全コンポーネントに加算されるオフセット |
| zOrder | Canvas 内での描画順序（昇順 = 奥から前） |

**主なメソッド**

```cpp
void Update(const Vector2& parentOffset);
void Draw() const;
void UpdateInteraction(const Vector2& mousePos, bool mousePressed, bool mouseTrigger);
IComponent* FindComponent(const std::string& name);
```

---

### Element — 最小単位コンポーネント

`Engine/include/Ui/Element.hpp`

単一 Sprite のラッパ。IComponent を実装する。
イベントを持つ場合のみ `IsInteractive()` が true を返し、Canvas からホバー状態を受け取る。

**Data 構造体（JSON 保存対象）**

```cpp
struct Data {
    std::string texture   = "white_x16.png";
    bool        visible   = true;
    Vector2     position  = {};
    Vector2     size      = {64.f, 64.f};
    Vector4     color     = {1,1,1,1};
    Vector2     textureLeftTop = {};
    Vector2     textureSize    = {};

    // ホバー視覚パラメータ
    Vector4 hoverColor  = {1,1,1,0.3f};  // ホバー時の色オーバーレイ
    float   hoverScale  = 1.0f;           // ホバー時スケール倍率
    Vector2 hoverOffset = {};             // ホバー時位置オフセット
};
```

---

### Button — ホバー視覚フィードバック付きコンポーネント

`Engine/include/Ui/Button.hpp`

背景 Sprite（ハイライト用）と前景 Sprite（アイコン・テクスチャラベル）の 2 枚で構成される。
テキスト表示は前景に事前レンダリングされたテクスチャを使用する。

**ホバー時の動作**
- 背景: `hoverColor` のアルファを上げて半透明ハイライトを表示
- 前景: `hoverScale` 倍スケール + `hoverOffset` 分オフセット

**主なパラメータ（JSON 保存）**

| パラメータ | デフォルト | 説明 |
|-----------|-----------|------|
| Background | - | 背景 Sprite の Texture / Position / Size / Color |
| Foreground | - | 前景 Sprite の Texture / Position / Size / Color |
| HoverColor | (1,1,1,0.3) | ホバー時に背景に適用する色 |
| HoverScale | 1.05 | ホバー時の前景スケール倍率 |
| HoverOffset | (0, -2) | ホバー時の前景位置オフセット |

---

## Canvas の入力処理

`Ui::Canvas` が `Singleton<Input>` からマウス座標・ボタン状態を取得し、
`IsInteractive()` が true のコンポーネントにのみ状態を配布する。

```
Canvas::Update()
 ├── Input からマウス座標・ボタン状態を取得
 └── 各 Layer に UpdateInteraction(mousePos, pressed, trigger) を渡す
      └── Layer が Interactive なコンポーネントに SetHovered / SetPressed を呼ぶ

Execute の発火:
 Canvas が pressedComponent_ を保持。
 マウスリリース時、同コンポーネント上ならば EventSystem.Execute(actionKey) を呼ぶ。
```

---

## Canvas の外部 API

Game 側からは名前ベースでコンポーネントにアクセスする。

```cpp
// コンポーネント検索（全 Layer 横断）
IComponent* FindComponent(const std::string& name) const;
Layer*      FindLayer(const std::string& layerName) const;

// アクション（変更なし）
void RegisterAction(const std::string& actionKey, const std::function<void()>& action);
void ExecuteAction(const std::string& actionKey);

// 状態（変更なし）
void SetActive(bool active);
bool IsActive() const;
bool IsDirty() const;
```

---

## JSON フォーマット

### Canvas ファイル（`Assets/Data/UI/<name>.json`）

```json
{
  "Position": [0.0, 0.0],
  "Layers": [
    {
      "Name": "Background",
      "Active": true,
      "PositionOffset": [0.0, 0.0],
      "ZOrder": 0,
      "Components": [
        {
          "Type": "Element",
          "Name": "bg",
          "Texture": "bg.png",
          "Position": [640.0, 360.0],
          "Size": [1280.0, 720.0],
          "Color": [1.0, 1.0, 1.0, 1.0]
        }
      ]
    },
    {
      "Name": "Interactive",
      "Active": true,
      "PositionOffset": [0.0, 0.0],
      "ZOrder": 1,
      "Components": [
        {
          "Type": "Button",
          "Name": "StartBtn",
          "Background": { "Texture": "btn_hl.png", "Position": [640, 360], "Size": [200, 60], "Color": [1,1,1,0] },
          "Foreground": { "Texture": "btn.png",    "Position": [640, 360], "Size": [200, 60], "Color": [1,1,1,1] },
          "HoverColor":  [1.0, 1.0, 1.0, 0.3],
          "HoverScale":  1.05,
          "HoverOffset": [0.0, -2.0],
          "Events": { "Click": "Pause.Resume" }
        }
      ]
    }
  ]
}
```

### Component の Type 一覧

| Type | クラス | 説明 |
|------|--------|------|
| `"Element"` | `Ui::Element` | 単一 Sprite |
| `"Button"` | `Ui::Button` | bg + fg Sprite のボタン |
| (将来) `"Slider"` | `Ui::Slider` | ドラッグ値入力 |
| (将来) `"Toggle"` | `Ui::Toggle` | オン/オフ切り替え |
| (将来) `"ProgressBar"` | `Ui::ProgressBar` | 値に応じた塗り |

---

## 将来の Component 追加ガイド

### Slider
- track (Element) + handle (Element) で構成
- Canvas が `draggedComponent_` を保持し、ドラッグ中は mouseDelta をコンポーネントに渡す
- 値変化時に Execute アクションを発火

### Toggle
- background + indicator (Element) で構成
- 押下で内部 bool を反転し、変化時に Execute アクションを発火

### ProgressBar
- background + fill (Element) で構成
- `SetValue(float 0~1)` で fill の textureSize か Size を制御
- 読み取り専用（インタラクションなし）

### Panel / Group
- IComponent 内部に Layer を持つか、Layer 自体をグループ用途に再利用するかは実装時に判断する

---

## 設計レビュー（AI 分析）

### 懸念点

#### [高] `IComponent::Debug()` がインターフェースに混入している

```cpp
virtual void Debug(const std::vector<std::string>& availableActions) = 0;
```

デバッグ機能は純粋仮想として全コンポーネントに強制するべきでない。
SRP 違反 + `#ifdef _DEBUG` で隠すべき関心事がインターフェースに漏れている。

**改善案:** `Debug()` を純粋仮想から外し、デフォルト実装 `{}` に変更。またはビルド設定で切り離す。

---

#### [高] `FindComponent()` が生ポインタを返す

```cpp
IComponent* FindComponent(const std::string& name) const;
```

返した後に Layer がコンポーネントを削除すると dangling pointer になる。
Game 側がポインタをキャッシュすることを前提にすると危険。

**改善案:** `std::optional<std::reference_wrapper<IComponent>>` か、コンポーネント操作を Canvas 経由のみにするクローズドAPIパターンを採用する。
短期的には生ポインタのまま「キャッシュ禁止」をドキュメントに明記することも許容できるが、将来的には要改善。

---

#### [中] Button の hover パラメータが Element に混入している

```cpp
// Element::Data 構造体に
Vector4 hoverColor  = {1,1,1,0.3f};
float   hoverScale  = 1.0f;
Vector2 hoverOffset = {};
```

`Element` は「単一 Spriteのラッパ」と定義されているのに、インタラクション用パラメータが入っている。
`IsInteractive()` が false の Element にも hoverColor/hoverScale が存在する状態になる。

**改善案:** hover パラメータを `Element::Data` から切り出し、`InteractiveData` または `Button` 固有の構造体に移動する。

---

#### [中] Canvas が Input をシングルトンから直接取得している

Canvas が `Singleton<Input>` に直接依存すると、テスタビリティがなく Input システムの変更が Canvas に波及する。

**改善案:** `Canvas::Update(const InputState& input)` として外側から渡す。
`InputState` は単純な構造体（mousePos, pressed, trigger）で十分。
Canvas が Input に依存しなくなり、コントローラー対応や将来の入力抽象化もしやすくなる。

---

#### [中] ZOrder のソート保証が不明

ドキュメントには「ZOrder 昇順で描画」とあるが `vector<Layer>` の順序管理について記述がない。
追加順に依存すると、後から Layer を追加したときに意図しない描画順になる可能性がある。

**改善案:** Layer 追加時または描画前に ZOrder でソートすることを明記するか、`std::multimap<int, Layer>` を採用する。

---

#### [低] `GetActionKey()` の返り値が `const std::string&`

```cpp
virtual const std::string& GetActionKey(EventKey event) const;
```

`HasEvent(event)` が false のとき何が返るか不明（空文字列への参照か UB か）。

**改善案:** `std::optional<std::string>` を返すか、`HasEvent` が true であることを事前条件として明確にドキュメント化する。

---

### 改善優先度まとめ

| 優先度 | 箇所 | 改善案 |
|--------|------|--------|
| 高 | `Debug()` 純粋仮想 | デフォルト実装 `{}` に変更 |
| 高 | `FindComponent` 生ポインタ | キャッシュ禁止明記 or API クローズド化 |
| 中 | `Element::Data` の hover パラメータ | インタラクション系を分離 |
| 中 | Input シングルトン直接参照 | `InputState` 構造体を引数で受け取る |
| 中 | ZOrder のソート保証 | 追加時ソートまたは設計明記 |
| 低 | `GetActionKey` の未定義動作 | `std::optional` または前提条件明記 |
