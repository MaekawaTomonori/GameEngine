#ifndef SkinningState_HPP_
#define SkinningState_HPP_
#include <array>
#include <d3d12.h>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "Line.hpp"
#include "Math/Matrix.hpp"
#include "ReferencePtr.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"
#include "src/Model/Data/ModelData.hpp"
#include "src/Model/Skeleton/Skeleton.hpp"

class Mesh;
class DirectXAdapter;
class ModelCommon;

const uint32_t MAX_INFLUENCE = 4;

/** @brief スキニングモデル専用の状態
 * 骨格・アニメーション・スキンクラスターを保持する。
 * スキニングデータを持つModelのみが生成する（内部コンポーネント）。
 */
class SkinningState {
    /** @brief 頂点のスキニング影響情報
     */
    struct VertexInfluence {
        std::array<float, MAX_INFLUENCE> weights;
        std::array<int32_t, MAX_INFLUENCE> jointIndices;
    };

    /** @brief GPU用のスキニングデータ
     */
    struct WellForGpu {
        Matrix4x4 space;
        Matrix4x4 inverseTranspose;
    };

    /** @brief スキンクラスターデータ
     * アニメーション用のボーン変換情報を保持
     */
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

    GESTD::ReferencePtr<DirectXAdapter> adapter_ = nullptr;
    GESTD::ReferencePtr<ModelCommon> common_ = nullptr;
    GESTD::ReferencePtr<ModelData> data_ = nullptr;

    Skeleton pose_;
    SkinCluster skinCluster_;

    bool animationEnable_ = true;
    bool animationTimerLock_ = true;
    float animationTime_ = 0.0f;

    /** デバッグ用ジョイント可視化ライン
     */
    Line line_;

public:
    /** @brief スキニング状態を初期化
     * @param _adapter DirectXアダプター
     * @param _common ModelCommon（SRVManager取得用）
     * @param _data モデルデータ（スケルトン・アニメーション・スキンクラスター元データ）
     * @param _mesh スキンクラスター生成先のメッシュ
     */
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<ModelCommon>& _common, const GESTD::ReferencePtr<ModelData>& _data, Mesh& _mesh);

    /** @brief アニメーション・骨格・スキンクラスター・デバッグ用ラインの更新
     */
    void Update();

    /** @brief デバッグ情報の表示（骨格ツリー・アニメーション操作）
     * @param _uuidPrefix ImGui IDの重複回避用プレフィックス
     */
    void Debug(const std::string& _uuidPrefix);

    /** @brief デバッグ用ジョイントラインを描画
     */
    void DrawLine() const;

    /** @brief スキンパレットのGPUディスクリプタハンドルを取得
     */
    D3D12_GPU_DESCRIPTOR_HANDLE GetPaletteHandle() const {
        return skinCluster_.paletteHandle.second;
    }

private:
    void CreateSkinCluster(Mesh& _mesh);
    void SetBindPose();
    void UpdateSkinCluster();
    void UpdateSkeleton();
    void UpdateAnimation();
    void ApplyAnimation();
    void CreateLine();
}; // class SkinningState

#endif // SkinningState_HPP_
