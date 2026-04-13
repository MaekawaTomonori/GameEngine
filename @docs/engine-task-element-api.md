# Engine Task: Element 基底クラスへのスプライト操作API追加

## 背景

`SpriteElement` のリファクタリングにより、旧 `Element::GetData()` / `SetData()` が廃止され、
スプライト固有のデータが `SpriteElement::GetSpriteData()` / `SetSpriteData()` に移された。

この結果、Game側コード（`KillCounter`, `SkillTree` 等）が `SpriteElement` の存在を
直接知らなければ操作できなくなっており、依存方向のルール違反になっている。

> Game は Engine の内部実装（具体的なElement型）を知るべきではない。
> Game は `Element*` (基底クラス) の公開APIのみを通じて操作する。

## 要求する変更

`Engine/include/Ui/Element.hpp` の `Element` 基底クラスに以下の仮想メソッドを追加する。

```cpp
/** @brief テクスチャを設定する（SpriteElement で有効）*/
virtual void SetTexture(const std::string& _tex) {}

/** @brief テクスチャのUVクリップ領域を設定する（SpriteElement で有効）
 * @param _leftTop テクスチャ左上座標（ピクセル）
 * @param _size    クリップサイズ（ピクセル）。{0,0} でテクスチャ全体
 */
virtual void SetTextureRegion(const Vector2& _leftTop, const Vector2& _size) {}
```

デフォルト実装は何もしない（空ボディ）。`SpriteElement` でオーバーライドして
内部の `data_.texture` / `data_.textureLeftTop` / `data_.textureSize` に反映する。

## SpriteElement 側の実装例

```cpp
// SpriteElement.hpp に override 宣言を追加
void SetTexture(const std::string& _tex) override;
void SetTextureRegion(const Vector2& _leftTop, const Vector2& _size) override;

// SpriteElement.cpp
void SpriteElement::SetTexture(const std::string& _tex) {
    data_.texture = _tex;
}

void SpriteElement::SetTextureRegion(const Vector2& _leftTop, const Vector2& _size) {
    data_.textureLeftTop = _leftTop;
    data_.textureSize    = _size;
}
```

## 変更してはいけないこと

- `GetSpriteData()` / `SetSpriteData()` はそのまま残す（Editorなど内部利用がある）
- `Element` の他のメソッドには触れない
- `SetColor()` / `SetVisible()` はすでに基底にあるため変更不要

## この変更後にGame側でできるようになること

```cpp
// SpriteElement を知らなくてよい
auto* elem = ui_->FindElementByName("current_d0");
if (!elem) return;
elem->SetTexture("numbers.png");
elem->SetTextureRegion({static_cast<float>(digit) * 64.f, 0.f}, {64.f, 96.f});
elem->SetVisible(true);
```

## 影響範囲

- `Engine/include/Ui/Element.hpp`
- `Engine/include/Ui/SpriteElement.hpp`
- Engine内の `SpriteElement.cpp`（実装追加のみ）
- Game側コードは **この変更後に別途修正する**（今回のタスク外）
