#ifndef Model_HPP_
#define Model_HPP_
#include <array>
#include <span>

#include "Math/Matrix.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Mesh/Mesh.hpp"
#include "src/Model/Common/ModelCommon.hpp"

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
        std::vector<Matrix4x4> bindPoseMatrices;

        Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
        D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
        std::span<VertexInfluence> mappedInfluence;

        Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
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

    float animationTime_ = 0.0f;

    ///GPU RESOURCES
    // world transform
    Microsoft::WRL::ComPtr<ID3D12Resource> wr_;
    Transformation* wd_ = nullptr;

    //Camera
    Microsoft::WRL::ComPtr<ID3D12Resource> cr_;
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
    void UpdateSkinCluster();
    void UpdateSkeleton();
    void ApplyAnimation() const;
}; // class Model

#endif // Model_HPP_