# Text システム再設計仕様書

## 背景・目的

現行の `Text` クラスはゲームコードが実体を所有し、毎フレーム `Draw()` を呼ぶ必要があった。
本再設計では `TextCommon` が全実体を管理する構造へ移行し、以下を実現する。

- キーを渡すだけで使えるゼロコスト API
- エディター上での TextBox 追加・編集・保存
- `Assets/Data/Text/texts.json` への永続化（Release ビルドでも有効）
- 同一キーへの複数ハンドル（全て同じ実体を操作）
- シーン切替時の自動全破棄

---

## 設計決定一覧

| # | 項目 | 決定 |
|---|------|------|
| Q1 | ラッパー | 新規ラッパーとして `Text` クラスを全面改修（名前維持） |
| Q2 | ライフタイム | TextCommon が実体所有。`Destroy(key)` / `DestroyAll()` で破棄 |
| Q3/Q10 | 永続化 | JSON (`Assets/Data/Text/texts.json`) |
| Q4 | 互換性 | 破壊的変更 OK |
| Q5 | アクセス | ゲームコード・エディター両方から操作可 |
| Q6 | ハンドル | 軽量・コピー可・非所有（キー文字列を保持するのみ） |
| Q7 | 検索キー | 文字列名。エディター命名と一致。未存在なら新規作成 |
| Q8 | 生成口 | `Text(key)` コンストラクタが FindOrCreate を呼ぶ |
| Q9 | 全破棄 | `DestroyAll()` で全エントリ消去 |
| Q11 | パス | `Assets/Data/Text/texts.json` |
| Q12 | 追加UI | `[+]` → `"TextBox_N"` で仮名追加・後でリネーム |
| Q13 | 重複ハンドル | 許容。同一実体を共有参照 |
| Q14 | API | コンストラクタにキーのみ渡し、他はメソッドチェーン |
| Q15 | Find | コンストラクタが自動で FindOrCreate |

---

## アーキテクチャ概観

```
Game Code
  Text score_("score");          ← 軽量ハンドル（キー文字列を保持するだけ）
  score_.SetText("Score: 100");  ← TextCommon の実体を更新

TextCommon (Singleton)
  unordered_map<string, TextData>  ← 全実体を所有
  Draw(Renderer*)                  ← 全可視エントリを自動描画（ゲームコードの Draw() 呼び出し不要）
  LoadFromJson() / SaveToJson()    ← nlohmann/json で直接読み書き

SceneSwitcher::Update()
  scene_->Finalize(); scene_.reset();
  Singleton<TextCommon>::GetInstance()->DestroyAll();  ← ここに追加

Editor (ImGui / DEBUG のみ)
  [+] ボタン → TextBox_N を先頭に仮名追加
  各エントリ: 名前リネーム・内容編集・位置・サイズ・色
  [Save] ボタン → texts.json へ書き出し
```

---

## クラス設計

### TextData（TextCommon 内部構造体）

```cpp
struct TextData {
    std::string text     = "";
    Vector2     position = {};
    float       fontSize = 32.f;
    Vector4     color    = {1.f, 1.f, 1.f, 1.f};
    bool        visible  = true;
};
```

### TextCommon の変更点

```cpp
// ストレージ変更
// Before: std::vector<Text*> texts_;
// After:  std::vector<std::string> order_;                  ← 挿入順（エディター表示用）
//         std::unordered_map<std::string, TextData> entries_;

// 追加メソッド
TextData&   FindOrCreate(const std::string& key);
void        Destroy(const std::string& key);
void        DestroyAll();
void        SaveToJson();
void        LoadFromJson();

// 削除
// void RegisterText(Text*);
// void UnregisterText(Text*);

// 変更
// Draw(Renderer*): entries_ を全件走査して SubmitEntry を自動呼び出し

static constexpr std::string_view kJsonDir  = "Assets/Data/Text/";
static constexpr std::string_view kJsonFile = "Assets/Data/Text/texts.json";
```

### Text（ラッパーハンドル）

```cpp
class Text {
    std::string                     key_;
    GESTD::ReferencePtr<TextCommon> common_;

public:
    /** @brief キーで TextCommon の実体を FindOrCreate する
     *  JSON 読み込み済みならその値を使用、未登録ならデフォルト値で新規作成 */
    explicit Text(const std::string& key);

    // ─── Setters（メソッドチェーン対応）───────────────────────────
    Text& SetText    (const std::string& text);
    Text& SetPosition(float x, float y);
    Text& SetFontSize(float size);
    Text& SetColor   (const Vector4& color);
    Text& SetVisible (bool visible);

    // ─── Getters ─────────────────────────────────────────────────
    const std::string& GetText()     const;
    Vector2            GetPosition() const;
    float              GetFontSize() const;
    const Vector4&     GetColor()    const;
    bool               IsVisible()   const;

    /** @brief TextCommon にキーが存在するか（DestroyAll 後は false） */
    bool IsValid() const;
};
```

**`Initialize()` と `Draw()` は廃止。TextCommon::Draw() が全件自動描画する。**

---

## JSON フォーマット

**パス**: `Assets/Data/Text/texts.json`

```json
{
  "textboxes": [
    {
      "name":     "score",
      "text":     "Score: 0",
      "x":        20.0,
      "y":        20.0,
      "fontSize": 32.0,
      "color":    [1.0, 1.0, 1.0, 1.0],
      "visible":  true
    },
    {
      "name":     "title",
      "text":     "GAME OVER",
      "x":        400.0,
      "y":        300.0,
      "fontSize": 64.0,
      "color":    [1.0, 0.0, 0.0, 1.0],
      "visible":  false
    }
  ]
}
```

### JsonParams を使わない理由

既存の `JsonParams` は Group/Key/Value のパラメーター調整用途に特化しており、以下の理由で TextCommon には不向き：

- `bool` 型が `variant` に含まれない（int32_t で代替が必要）
- `Load()` はファイル不在時に `assert` でクラッシュする
- `Save()` 後に `datas_.erase()` されメモリから消える
- テキスト文字列の動的変更との相性が悪い

→ `nlohmann/json` を直接使用し、TextCommon 内で完結させる

---

## ゲームコード使用例

```cpp
// ─── 宣言（クラスメンバー）───────────────────────────────────────────
Text score_("score");    // JSON に "score" があれば読み込み値、なければ新規作成
Text title_("title");

// ─── 初期設定（JSON に値がない場合のデフォルトを上書き）────────────
score_.SetPosition(20.f, 20.f).SetFontSize(32.f);
title_.SetText("GAME OVER")
      .SetPosition(400.f, 300.f)
      .SetFontSize(64.f)
      .SetColor({1.f, 0.f, 0.f, 1.f});

// ─── 毎フレーム更新（Draw() 呼び出し不要）───────────────────────────
score_.SetText("Score: " + std::to_string(score_));

// ─── 同一キーへの複数ハンドル ────────────────────────────────────────
Text a("info");
Text b("info");       // 同じ実体を参照
a.SetText("Hello");
// b.GetText() == "Hello"  → true

// ─── DestroyAll 後のハンドル挙動 ─────────────────────────────────────
// score_.IsValid() == false
// score_.SetText("...") は何もしない（クラッシュしない）
```

---

## エディター UI 仕様

### TextBoxes CollapsingHeader

```
▼ TextBoxes                              [+]
──────────────────────────────────────────
Count: 3                            [Save]
──────────────────────────────────────────
☑  ▶ "Score: 0"        (score)
☑  ▶ "GAME OVER"       (title)
☑  ▶ ""                (TextBox_3)   ← 追加直後（先頭に挿入）
```

- `[+]` ボタン: ヘッダー行右端に配置（`ImGui::SameLine` + `ImGui::Button`）
- `Count:` 右に `[Save]` ボタン
- 各行は `ImGui::Checkbox`（visible）+ `ImGui::TreeNode`（プレビュー文字列）

### 各エントリの TreeNode 展開内容

```
  Name      [ TextBox_3      ]   ← InputText でリネーム可
  ─────────────────────────────
  Text      [                ]
  Position  X [  0.0 ] Y [  0.0 ]
  Font Size [ 32.0 ]
  Color     [■■■■          ]  (ColorEdit4)
```

### 操作定義

| 操作 | 挙動 |
|------|------|
| `[+]` ボタン | `"TextBox_N"` (N=現在の entries_ サイズ) で TextData 新規作成、`order_` の先頭に挿入 |
| Name フィールド Enter 確定 | 旧キー削除 → 新キーで再登録。衝突時は `"name_1"`, `"name_2"` ... でサフィックス付与 |
| `[Save]` ボタン | 全 entries_ を `Assets/Data/Text/texts.json` へ書き出し |
| 起動時 `Initialize()` | JSON が存在すれば自動ロード。なければ空で開始 |

---

## SceneSwitcher への組み込み

`SceneSwitcher::Update()` の場面切替ブロック（`scene_->Finalize()` の直後）に追加：

```cpp
// 既存コード
scene_->Finalize();
scene_.reset();
scene_ = nullptr;
Singleton<CameraDirector>::GetInstance()->Stop();
Singleton<LightManager>::GetInstance()->ClearRef();
if (context_.particle) {
    context_.particle->ClearActive();
}
// ↓ 追加
Singleton<TextCommon>::GetInstance()->DestroyAll();
```

`SceneSwitcher::Context` への追加は不要（Singleton 経由でアクセス可能なため）。

---

## 実装順序

```
Step 1  TextCommon: TextData 構造体を追加
Step 2  TextCommon: entries_ / order_ に変更、FindOrCreate / Destroy / DestroyAll を実装
Step 3  TextCommon: Draw(Renderer*) を entries_ 全件自動 SubmitEntry に変更
Step 4  TextCommon: LoadFromJson / SaveToJson を nlohmann/json で実装
Step 5  TextCommon: RegisterText / UnregisterText を削除
Step 6  TextCommon: エディター UI に [+] / リネーム / [Save] を追加
Step 7  Text クラス: ハンドルに全面改修（Initialize・Draw・Register/Unregister 削除）
Step 8  SceneSwitcher: Update() に DestroyAll() を追加
Step 9  動作確認: エディターで追加→Save→再起動でロード、シーン切替で消去
```

---

## 変更ファイル一覧

| ファイル | 変更種別 |
|---------|---------|
| `src/Text/TextCommon.hpp` | TextData 追加、メソッド追加・削除 |
| `src/Text/TextCommon.cpp` | 全面変更 |
| `include/Text/Text.hpp` | ハンドル API に全面変更 |
| `src/Text/Text.cpp` | 全面変更 |
| `src/Scene/SceneSwitcher.cpp` | DestroyAll() 追加（1行） |
