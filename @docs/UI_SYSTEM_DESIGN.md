# UI_SYSTEM_DESIGN.md

## 目的

本システムは、ゲームエンジン上で動作する軽量UIフレームワークを構築することを目的とする。  
特に以下を満たすことを重視する。

- Webフロントエンジニアが直感的に扱える記法
- ゲームエンジンに適した軽量な実行コスト
- C++エンジンとの明確な責務分離
- 宣言的UIによる開発効率向上
- Editorとは独立した運用が可能

本システムは「ブラウザの再現」ではなく、Webライクな操作性の再設計を目的とする。

---

## 設計思想

### Webから借りるもの
- Flexbox的レイアウト
- CSS風スタイル
- 疑似状態（hover / pressed / focus）
- トランジション / アニメーション
- 宣言的UI

### Webから捨てるもの
- フルCSS互換
- DOM完全再現
- JavaScript実行環境
- ブラウザ互換地獄

---

## 構成

- Markup
- Style
- Layout
- Event
- Animation
- Binding
- Renderer
- Bridge

---

## マークアップ例

<Panel id="menu">
    <Text value="MENU"/>
    <Button onClick="StartGame"/>
</Panel>

---

## スタイル例

Button {
    height: 48px;
    background-color: #333;
}

Button:hover {
    background-color: #555;
}

---

## イベント

<Button onClick="StartGame"/>

C++側でActionにマッピングする。

---

## アニメーション

transition と keyframes のみ対応する軽量設計とする。

---

## バインディング

<Text value="{player.hp}"/>

---

## レンダラ

- Quad
- Text
- Atlas
- Batch

---

## 原則

- 軽量
- 予測可能
- 宣言的
- Web風だがWebではない

---

## 目標

Web経験者が即扱え、ゲーム用途に最適化されたUIシステムを実現する。
