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
class Model{
    struct VertexInfluence {
        std::array<float, MAX_INFLUENCE> weights;
        std::array<int32_t, MAX_INFLUENCE> jointIndices;
    };
    struct WellForGpu {
        Matrix4x4 space;
        Matrix4x4 inverseTranspose;
    };
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

public:
    Model();
    void Initialize(const std::string& _name);
    void Update();
    void Draw() const;

private:
    void Load(const std::string& _name) const;
    void Debug();
    void UpdateMapData() const;
    void CreateSkinCluster();
    void SetBindPose(Skeleton& _skeleton);
    void UpdateSkinCluster();
    void UpdateSkeleton() const;
    void UpdateAnimation();
    void ApplyAnimation() const;

    void CreateLine();
    void DrawLine() const;
}; // class Model

#endif // Model_HPP_