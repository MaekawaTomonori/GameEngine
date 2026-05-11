# PointLight Shadow 実装方針共有メモ

## 目的

現在開発中の自作C++ / DirectX 12ゲームエンジンにおいて、3Dオブジェクトに対するPointLightの影表現を実装したい。

ただし、単純にリアルなOmnidirectional Shadow Mapを全PointLightへ適用するのではなく、ゲーム全体として軽量に動作する設計を重視したい。

理想は、原神のようにスマートフォンでも滑らかに動作しながら、見た目の品質が高いグラフィック表現である。

そのため、完全な物理正確性よりも、ゲームとして自然に見え、処理負荷が低く、エンジンとして運用しやすい方式を優先したい。

---

## 現在の描画構成

現在はForward Renderingで3Dモデルを描画している。

光源計算は主に `Model.PS.hlsl` のPixel Shader内で行っている。

ただし、影は他のオブジェクトにも影響するため、ModelのPixel Shader内だけで完結するものではないと認識している。

PointLightの影をリアルに行う場合、最低でも以下のような構成が必要になると考えている。

```txt
1. ShadowMap生成Pass
2. 通常Model描画Pass
3. Model.PS.hlslでShadowMapを参照して減光
```

PointLightの場合、リアル寄りにするならCube Shadow Mapを使い、1つのPointLightにつき6方向のShadowMap描画が必要になる。

```txt
PointLight 1個 = ShadowMap 6面分の描画
```

そのため、全PointLightにリアルタイム影を付ける設計は避けたい。

---

## 現在のPointLight構造体

現在のPointLightは以下の構成。

```cpp
struct PointLight {
    Vector4 color;

    Vector3 position;
    float intensity;

    float radius;
    float decay;
    float pad[2];
};
```

この構造体自体はPointLightとして十分だと考えている。

影に関する情報は、全PointLightに混ぜ込むのではなく、影付きPointLight専用の別構造体に分離したい。

---

## 実装思想

### 1. 軽量化を最優先する

今後、ポストエフェクトなど複数の重い描画処理を追加する可能性がある。

そのため、PointLight Shadowだけで描画負荷を大きく使い切る設計にはしたくない。

特に避けたいものは以下。

```txt
- 全PointLightにCube Shadow Mapを持たせる
- 毎フレーム全影付きPointLightの6面ShadowMapを更新する
- Pixel Shader内で大量のPointLightを全ピクセルに対して総当たり計算する
- 高解像度ShadowMapを標準にする
- 高サンプル数のPCF（16サンプル以上）を標準にする
```

---

### 2. 影付きPointLightは特別扱いにする

通常のPointLightは影なしで扱い、重要なライトだけ影付きにしたい。

```txt
通常PointLight:
    影なし

重要PointLight:
    Cube Shadow Mapを使用（最大1個）
```

最初の実装では、影付きPointLightは最大1個でよい。

```cpp
static constexpr uint32_t MaxShadowPointLights = 1;
```

必要になった場合のみ2個以上へ拡張する。

---

### 3. PointLight本体とShadow情報は分離する

影なしPointLightの方が圧倒的に多い想定のため、PointLight本体にShadow用の情報を混ぜたくない。

想定している分離例。

```cpp
struct PointLightShadow {
    uint32_t lightIndex;
    uint32_t shadowMapIndex;

    float bias;
    float strength;

    uint32_t resolution;
    uint32_t updateInterval;
    uint32_t lastUpdatedFrame;
    uint32_t pad;
};
```

PointLight本体は光源情報のみを持つ。

---

## 軽量化のために取り入れたい方針

### 1. ShadowMap解像度は低めから始める

PointLight Shadowは6面必要なため、解像度を高くしすぎない。

```txt
256 x 256 x 6:
    軽量。最初の基準

512 x 512 x 6:
    品質が必要な場合

1024 x 1024 x 6:
    基本的には避ける
```

最初は `256` または `512` を検討したい。

---

### 2. ShadowMapを毎フレーム更新しない

静的ライト、静的遮蔽物の場合はShadowMapをキャッシュしたい。

```txt
ライトが動いていない
影を落とすオブジェクトが動いていない
    → ShadowMapを再生成しない
```

動的な場合でも、毎フレームではなく数フレームに1回の更新を検討したい。

```cpp
bool needUpdate =
    lightMoved ||
    shadowCasterMoved ||
    forceUpdate ||
    frameIndex % updateInterval == 0;
```

---

### 3. ShadowCasterを絞る

ShadowPassで全モデルを描画しない。

```cpp
if (!model.castShadow) continue;
if (!Intersects(model.bounds, lightSphere)) continue;
```

PointLightの半径外にあるモデルはShadowPassから除外する。

---

### 4. Pixel Shader内のPointLight計算を軽くする

現在Forward Renderingなので、Pixel Shader内のPointLight計算負荷も重要。

避けたい処理。

```hlsl
length()              // sqrt不要。distSqで代替する
大量のPointLightループ
CubeMap Shadowの多回サンプリング
```

距離判定は `distSq` ベースで行い、範囲外は早期スキップする。
減衰は `decay` を使った `pow()` で制御する（実装済み）。

```hlsl
float3 toLight = light.position - worldPos;
float distSq = dot(toLight, toLight);
float radiusSq = light.radius * light.radius;

if (distSq >= radiusSq) {
    continue;
}

float atten = pow(saturate(1.0f - distSq / radiusSq), light.decay);

float3 L = toLight * rsqrt(max(distSq, 0.0001f));
float NdotL = saturate(dot(normal, L));
```

---

### 5. モデルごとに影響ライトを絞る

Forward Renderingのままなら、全PointLightを全ピクセルで回すのは避けたい。

CPU側でモデルごとに影響するPointLightを選び、`Model.PS.hlsl` へ渡すライト数を制限したい。

```cpp
static constexpr uint32_t MaxLightsPerModel = 8;
```

モデルのBoundingSphere/AABBとPointLightのSphereが交差するものだけを候補にする。

```cpp
bool IsAffectingModel(const PointLight& light, const BoundingSphere& modelSphere) {
    float r = light.radius + modelSphere.radius;
    return LengthSq(light.position - modelSphere.center) <= r * r;
}
```

候補が多い場合は、距離・強度・重要度でスコアリングして上位だけを使う。

```txt
score = intensity * overlapFactor / distanceToModel
```

---

## 影表現の分類

### A. 影なしPointLight

大半のPointLightはこれでよい。

対象例。

```txt
- 装飾ライト
- 遠距離ライト
- 小さいライト
- 大量に置かれるライト
```

---

### B. Cube Shadow Map

本当に必要なPointLightだけに使用する。

対象例。

```txt
- 主役ライト
- ボス戦や演出用ライト
- プレイヤー周辺の重要な光源
- ホラーゲームの懐中電灯に近い重要光源
```

標準機能ではなく、高コストな特殊機能として扱いたい。

---

## 目指したい実装順序

以下の順で実装したい。

```txt
1. [完了] PointLight計算をdistSqベースに軽量化
2. [完了] decayを使ったpow()で減衰カーブを制御
3. PointLightShadow構造体をPointLight本体から分離して設計
4. 影付きPointLightを最大1個としてCube Shadow Mapを実装
5. ShadowMap解像度は256または512から開始
6. ShadowCasterの距離・フラグによる絞り込みを実装
7. PCFによるソフトシャドウを実装（4サンプル程度・ぼやけ幅を調整可能にする）
8. ShadowMapのキャッシュと更新頻度制御を実装
9. CPU側でモデルごとの影響PointLightを最大8個程度に絞る
```

---

## Codexに判断してほしいこと

以下を実際のコード構成を見たうえで判断してほしい。

```txt
1. 現在のForward Rendering構成で、どこから軽量化するのが最も効果的か
2. Model.PS.hlsl内のPointLight計算をどう修正するべきか
3. PointLightShadowをどのクラス・ファイルに分離するべきか
4. ShadowMap用のPassを既存の描画パイプラインにどう追加するべきか
5. Depth CubeMap用のリソース管理をどの既存クラスに組み込むべきか
6. ShadowCasterの絞り込みをどこで行うべきか
7. ShadowMap更新頻度制御をどこに持たせるべきか
8. 将来的にForward+ / Clustered Forwardへ拡張する場合、今の設計で邪魔になる部分はどこか
9. 原神のような「軽くて綺麗」な方向へ進めるため、今やるべきこと・後回しにするべきこと
```

---

## 重視する判断基準

```txt
- スマホでも動くような軽量設計の思想
- DirectX 12で管理しやすい構造
- 自作エンジンとして拡張しやすいこと
- 実装コストが高すぎないこと
- 見た目の費用対効果が高いこと
- 完全な物理正確性よりゲームとして自然に見えること
- 後からForward+ / Clustered Forwardへ拡張できる余地があること
```

---

## 現時点での結論

PointLightのリアル影は、全ライト標準ではなく、重要ライト専用の高コスト機能として扱いたい。

最初から完璧なPointLight Shadowを目指すのではなく、以下の組み合わせで進めたい。

```txt
通常PointLight:
    影なし・軽量減衰

重要PointLight:
    最大1個だけCube Shadow Map

静的ライト:
    ShadowMapをキャッシュ

動的ライト:
    更新頻度を制限

Pixel Shader:
    distSqベース・decay制御の減衰・ライト数制限
```

この方針を前提に、現在のコードベースを見て、最も安全で効果の高い実装順序と具体的な修正箇所を提案してほしい。
