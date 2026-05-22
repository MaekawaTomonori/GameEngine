DirectX 12製自作ゲームエンジンにおける Model 周辺の設計を行いたいです。
目的は、ユーザーが安全かつ簡単に扱える Model クラスを提供しつつ、内部では obj / glTF 読み込み、静的メッシュ、スキンメッシュ、アニメーション、半透明描画、影、カリング、将来的なインスタンシングに対応しやすい構造にすることです。

# 前提

- 言語は C++20
- 描画APIは DirectX 12
- モデル読み込みは Assimp を使用
- 対応形式は obj / glTF
- Game プロジェクトから Engine を使用する
- Engine は Game に依存してはいけない
- ユーザー側に DirectX 12 の詳細、Assimp、PSO、Descriptor、VertexBuffer、IndexBuffer を直接触らせない
- Model クラスはユーザー向けの安全なインターフェースとして扱う
- ロード済みデータは Repository で共有・キャッシュする
- PSO など描画共通物は ModelCommon または Pipeline 管理クラスに集約する
- 最初はアニメーション更新はメインスレッド同期でよい
- 将来的に AnimationSystem / JobSystem による非同期化を追加できる構造にしておく
- 透明描画は基本的に AlphaBlend を想定する
- ただし通常モデルは Opaque を標準とし、必要なものだけ AlphaBlend にする

# 設計方針

Model は巨大クラスにしない。
Model はユーザーが触るファサード/ハンドルとし、実体は ModelInstance / ModelResource / Mesh / Material / AnimatorInstance / ModelRenderer / ModelRepository に分離する。

望ましい全体構成は以下。

Model
- ユーザーが触る安全な窓口
- Transform 操作
- 表示/非表示
- 影設定
- Alpha 設定
- Animation 再生命令
- Update / Draw
- Assimp、PSO、Descriptor、GPUバッファを直接触らない

ModelInstance
- 1体ごとの状態
- Transform
- WorldMatrix
- WorldBounds
- MaterialOverride
- AnimatorInstance
- visible
- castShadow
- receiveShadow
- alpha
- alphaMode

ModelResource
- ロード済み共有データ
- Mesh 一覧
- Material 一覧
- Texture 参照
- Skeleton
- AnimationClip 一覧
- localBounds
- obj / glTF の違いはここに正規化する

Mesh
- StaticMesh / SkinnedMesh を区別する
- VertexBuffer
- IndexBuffer
- SubMesh 一覧
- localBounds
- GPU リソース管理

SubMesh
- indexStart
- indexCount
- materialIndex
- localBounds
- castShadow
- receiveShadow

Material
- baseColorTexture
- normalTexture
- metallicRoughnessTexture
- baseColor
- alpha
- alphaMode
- doubleSided
- receiveShadow

ModelRepository
- AssimpModelLoader を使って obj / glTF をロード
- 同じファイルを二重ロードしない
- ロード失敗時はエラーモデルを返す
- weak_ptr / shared_ptr などでキャッシュ管理する
- TextureRepository と連携する

AssimpModelLoader
- Assimp 依存を閉じ込める
- obj / glTF を読み、ModelResource に変換する
- ファイル形式差を ModelResource 側へ正規化する
- glTF の alphaMode, doubleSided, skeleton, animation を可能な範囲で読む

ModelCommon / ModelPipelineLibrary
- RootSignature
- PSO
- Shadow用PSO
- Static / Skinned
- Opaque / AlphaBlend
- 必要に応じて CullMode などの組み合わせで PSO を管理する
- obj用PSO / glTF用PSO のようにファイル形式別PSOにはしない

ModelRenderer
- Model::Draw() から Submit される
- 即Drawではなく描画キューに積む
- OpaqueQueue / AlphaBlendQueue / ShadowQueue を分ける
- Frustum Culling を行う
- Opaque は先に描画
- AlphaBlend は後で奥から手前にソートして描画
- ShadowPass は castShadow が true のものだけ描画
- 将来的に StaticMesh の Instancing に対応しやすくする

AnimatorInstance
- ModelInstance ごとのアニメーション状態
- currentAnimation
- nextAnimation
- currentTime
- nextTime
- blendTime
- blendDuration
- speed
- localPose
- globalPose
- matrixPalette
- Play / CrossFade / Update を持つ
- 最初は同期 Update でよい
- 将来的に AnimationSystem へ移せるよう Model から分離する

AnimationClip
- ModelResource 側に保持される共有データ
- Keyframe / Channel / duration / ticksPerSecond 等を持つ

Skeleton
- ModelResource 側に保持される共有データ
- Joint 一覧
- parent index
- inverseBindPose

# StaticMesh / SkinnedMesh

obj や静的 glTF にスキニング情報を持たせない。
無駄な jointIndices / jointWeights を StaticMeshVertex に入れない。

StaticMeshVertex:
- position
- normal
- tangent
- texcoord
- color

SkinnedMeshVertex:
- position
- normal
- tangent
- texcoord
- color
- jointIndices[4]
- jointWeights

分類:
- obj → StaticMesh
- glTF 静的 → StaticMesh
- glTF スキンあり → SkinnedMesh

PSOもファイル形式ではなく MeshType と AlphaMode で分ける。

必要最小限:
- StaticOpaque
- StaticAlphaBlend
- StaticShadow
- SkinnedOpaque
- SkinnedAlphaBlend
- SkinnedShadow

必要になれば追加:
- AlphaCutout
- ShadowAlphaCutout
- CullNone
- DepthOnly

# 透明描画

透明対応で重要なのは Opaque と AlphaBlend を分けること。

Opaque:
- 完全不透明
- Blend = OFF
- DepthTest = ON
- DepthWrite = ON
- 先に描く
- インスタンシングしやすい
- Early-Z が効く
- 通常モデルの標準

AlphaBlend:
- 半透明
- Blend = ON
- DepthTest = ON
- DepthWrite = OFF
- Opaque の後に描く
- 奥から手前にソートして描く
- インスタンシングは最初は無理に狙わない
- ガラス、水、フェード中モデルなどに使う

AlphaBlend 用 D3D12 設定例:
- SrcBlend = D3D12_BLEND_SRC_ALPHA
- DestBlend = D3D12_BLEND_INV_SRC_ALPHA
- BlendOp = D3D12_BLEND_OP_ADD
- SrcBlendAlpha = D3D12_BLEND_ONE
- DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA
- BlendOpAlpha = D3D12_BLEND_OP_ADD
- DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO
- DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL

やってはいけないこと:
- obj用透明PSO
- glTF用透明PSO
- モデルごとの透明PSO
- 透明度ごとのPSO
- 全モデルをAlphaBlend扱いする
- AlphaBlendでDepthWriteをONにする
- AlphaBlendをOpaqueより先に描く

Material設計例:
enum class AlphaMode
{
    Opaque,
    Blend
};

struct Material
{
    TextureHandle baseColorTexture;
    TextureHandle normalTexture;
    Vector4 baseColor = {1, 1, 1, 1};
    AlphaMode alphaMode = AlphaMode::Opaque;
    float alpha = 1.0f;
    bool doubleSided = false;
    bool receiveShadow = true;
};

ユーザーAPI例:
model.SetAlphaMode(AlphaMode::Blend);
model.SetAlpha(0.5f);

ただし、alpha < 1.0 だから自動で Blend にするかどうかは慎重に設計する。
安全性を優先するなら SetAlpha(0.5f) 時に AlphaMode::Blend へ自動変更してもよいが、内部ルールは明確にする。

# 影

ModelInstance に以下を持たせる。

- castShadow
- receiveShadow

通常描画:
- visible == true
- Camera Frustum に入っている
- Material / Instance の設定に従って描画

ShadowPass:
- castShadow == true
- Light Frustum に入っている
- Shadow 用 PSO で描画

AlphaBlend の影:
- 最初は castShadow = false 推奨
- 半透明モデルが濃い影を落とすと不自然になりやすい
- 必要になれば将来拡張

receiveShadow:
- Material または Instance で制御する
- PixelShader 側で ShadowMap 参照を行うかどうかを切り替える

# カリング

最低限必要:
- Back Face Culling
- Frustum Culling

Back Face Culling:
- RasterizerState の CullMode で行う
- doubleSided の Material は CullNone が必要

Frustum Culling:
- ModelInstance の worldBounds を使う
- Camera Frustum 外なら Submit または Draw しない
- ShadowPass では Light Frustum で判定する

Occlusion Culling:
- 初期実装では不要
- 将来的に Hi-Z / Occlusion Query / GPU Culling などを検討

# インスタンシング

Model::Draw() は即 DrawIndexedInstanced を呼ばない。
ModelRenderer に Submit するだけにする。

理由:
- Renderer 側で同じ Mesh / Material / PSO をまとめられる
- 将来的に StaticMesh を DrawIndexedInstanced で描ける
- Opaque はインスタンシングしやすい
- AlphaBlend はソート優先のため最初は個別 Draw でよい
- SkinnedMesh は MatrixPalette が個体ごとに違うので最初は個別 Draw でよい

RenderKey 例:
struct RenderKey
{
    MeshType meshType;
    Mesh* mesh;
    Material* material;
    PipelineType pipelineType;
};

# 同期アニメーション更新

最初は非同期化しない。
Model::Update(deltaTime) で同期的に AnimatorInstance を更新する。

流れ:
1. Model::Update(deltaTime)
2. ModelInstance の Transform 更新
3. AnimatorInstance::Update(deltaTime)
4. AnimationClip サンプリング
5. CrossFade 計算
6. LocalPose 作成
7. GlobalPose 作成
8. MatrixPalette 作成
9. WorldBounds 更新
10. Draw() で Submit

Animator API例:
class AnimatorInstance
{
public:
    void Play(std::string_view name);
    void CrossFade(std::string_view name, float fadeTime);
    void SetSpeed(float speed);
    void Update(float deltaTime);
    const std::vector<Matrix4x4>& GetMatrixPalette() const;
};

Model API例:
class Model
{
public:
    static Model Create(std::string_view path);

    void SetPosition(const Vector3& position);
    void SetRotation(const Quaternion& rotation);
    void SetScale(const Vector3& scale);

    void SetVisible(bool visible);
    void SetCastShadow(bool enable);
    void SetReceiveShadow(bool enable);

    void SetAlpha(float alpha);
    void SetAlphaMode(AlphaMode mode);

    void PlayAnimation(std::string_view name);
    void CrossFadeAnimation(std::string_view name, float fadeTime);
    void SetAnimationSpeed(float speed);

    void Update(float deltaTime);
    void Draw();

private:
    std::shared_ptr<ModelInstance> instance_;
};

# 安全性

ユーザーが雑に使っても落ちない設計にする。

- 存在しないモデルパスならエラーモデルを返す
- 存在しないテクスチャならデフォルトテクスチャを使う
- 存在しないアニメーション名なら何もしない、またはログ警告
- null resource で Draw しても落ちない
- Update なしで Draw しても破綻しにくい
- Skeleton がないモデルに PlayAnimation しても落ちない
- AlphaBlend の設定ミスはログで警告できるようにする

# 欲しい出力

以下を提案してください。

1. 推奨クラス構成
2. 各クラスの責務
3. ヘッダレベルの C++ クラス案
4. ModelRepository と AssimpModelLoader の流れ
5. ModelRenderer の Submit / Flush の流れ
6. Opaque / AlphaBlend の描画順
7. StaticMesh / SkinnedMesh の分離方法
8. 同期 AnimatorInstance の設計
9. ShadowPass の設計
10. 将来的な Instancing / 非同期Animation / RayTracing への拡張余地
11. 実装順序
12. 避けるべき設計ミス

特に重視すること:
- Model はユーザー向けに簡単で安全
- 内部は責務分離する
- obj / glTF で PSO を分けない
- StaticMesh に不要な Skinning 情報を持たせない
- AlphaBlend 対応で PSO が無秩序に増えない
- Opaque と AlphaBlend を正しく分ける
- Draw 即実行ではなく Submit 方式にする
- 将来の Instancing に繋がる構造にする
- 最初は同期Animationでよいが、将来非同期化しやすい構造にする