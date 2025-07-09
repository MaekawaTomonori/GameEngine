#ifndef Model_HPP_
#define Model_HPP_
#include "Math/Matrix.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Mesh/Mesh.hpp"
#include "src/Model/Common/ModelCommon.hpp"

class Model{
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

    ///GPU RESOURCES
    // world transform
    Microsoft::WRL::ComPtr<ID3D12Resource> wr_;
    Transformation* wd_ = nullptr;

    //Camera
    Microsoft::WRL::ComPtr<ID3D12Resource> cr_;
    CameraForGpu* cd_ = nullptr;

public:
    Model();
    void Initialize(const std::string& _name);
    void Update();
    void Draw() const;

private:
    void Load(const std::string& _name) const;
    void Debug();
    void UpdateMapData() const;
}; // class Model

#endif // Model_HPP_