#ifndef Model_HPP_
#define Model_HPP_
#include <array>
#include <span>
#include <memory>

#include "Line.hpp"
#include "Math/Matrix.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Mesh/Mesh.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

const uint32_t MAX_INFLUENCE = 4;

/// <summary>
/// 3Dモデルクラス
/// メッシュ、テクスチャ、アニメーション、スキニングを管理
/// </summary>
class Model{
    /// <summary>
    /// 頂点のスキニング影響情報
    /// </summary>
    struct VertexInfluence {
        std::array<float, MAX_INFLUENCE> weights;
        std::array<int32_t, MAX_INFLUENCE> jointIndices;
    };

    /// <summary>
    /// GPU用のスキニングデータ
    /// </summary>
    struct WellForGpu {
        Matrix4x4 space;
        Matrix4x4 inverseTranspose;
    };

    /// <summary>
    /// スキンクラスターデータ
    /// アニメーション用のボーン変換情報を保持
    /// </summary>
    struct SkinCluster {
        std::vector<Matrix4x4> inverseBindPoses;

        std::unique_ptr<DX12Resource> influenceResource;
        D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
        std::span<VertexInfluence> mappedInfluence;

        std::unique_ptr<DX12Resource> paletteResource;
        std::span<WellForGpu> mappedPalette;
        uint32_t srvIndex;
        std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteHandle;
    };

    /// <summary>
    /// モデルの変換行列データ
    /// </summary>
    struct Transformation{
        Matrix4x4 wvp;
        Matrix4x4 world;
        Matrix4x4 inverse;
    };

    ModelCommon* common_ = nullptr;
    DirectXAdapter* adapter_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

    std::string uuid_;
    Transform transform_;
    ModelData* data_ = nullptr;
    std::unique_ptr<Mesh> mesh_;
    Camera* camera_ = nullptr;

    bool animationEnable_ = true;
    bool animationTimerLock_ = true;
    float animationTime_ = 0.0f;

    Line line_;

    ///GPU RESOURCES
    // world transform
    std::unique_ptr<DX12Resource> wr_;
    Transformation* wd_ = nullptr;

    //Camera
    std::unique_ptr<DX12Resource> cr_;
    CameraForGpu* cd_ = nullptr;

    SkinCluster skinCluster_;

    // Environment mapping
    std::string environmentTexture_ = "";

public:
    Model();

    /// <summary>
    /// モデルを初期化
    /// </summary>
    /// <param name="_name">モデル名</param>
    void Initialize(const std::string& _name);

    /// <summary>
    /// モデルの更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// モデルを描画
    /// </summary>
    void Draw() const;

    /// <summary>
    /// 平行移動を設定
    /// </summary>
    /// <param name="_translate">平行移動ベクトル</param>
    /// <returns>メソッドチェーン用の自身への参照</returns>
    Model& SetTranslate(Vector3 _translate);

    /// <summary>
    /// 回転を設定
    /// </summary>
    /// <param name="_rotate">回転ベクトル</param>
    /// <returns>メソッドチェーン用の自身への参照</returns>
    Model& SetRotate(Vector3 _rotate);

    /// <summary>
    /// スケールを設定
    /// </summary>
    /// <param name="_scale">スケールベクトル</param>
    /// <returns>メソッドチェーン用の自身への参照</returns>
    Model& SetScale(Vector3 _scale);

    /// <summary>
    /// 環境マッピング用テクスチャを設定
    /// </summary>
    /// <param name="_texture">テクスチャパス</param>
    /// <returns>メソッドチェーン用の自身への参照</returns>
    Model& SetEnvironmentTexture(const std::string& _texture);

    /// <summary>
    /// テクスチャを設定
    /// </summary>
    /// <param name="_texture">テクスチャパス</param>
    /// <returns>メソッドチェーン用の自身への参照</returns>
    Model& SetTexture(const std::string& _texture);

    /// <summary>
    /// モデルデータを事前読み込み
    /// </summary>
    /// <param name="_name">モデル名</param>
    static void Load(const std::string& _name);

private:
    /// <summary>
    /// デバッグ情報の表示
    /// </summary>
    void Debug();

    /// <summary>
    /// マップデータの更新
    /// </summary>
    void UpdateMapData() const;

    /// <summary>
    /// スキンクラスターの生成
    /// </summary>
    void CreateSkinCluster();

    /// <summary>
    /// バインドポーズの設定
    /// </summary>
    /// <param name="_skeleton">スケルトン</param>
    void SetBindPose(Skeleton& _skeleton);

    /// <summary>
    /// スキンクラスターの更新
    /// </summary>
    void UpdateSkinCluster();

    /// <summary>
    /// スケルトンの更新
    /// </summary>
    void UpdateSkeleton() const;

    /// <summary>
    /// アニメーションの更新
    /// </summary>
    void UpdateAnimation();

    /// <summary>
    /// アニメーションの適用
    /// </summary>
    void ApplyAnimation() const;

    /// <summary>
    /// デバッグ用ラインの生成
    /// </summary>
    void CreateLine();

    /// <summary>
    /// デバッグ用ラインの描画
    /// </summary>
    void DrawLine() const;
}; // class Model

#endif // Model_HPP_