#ifndef Model_HPP_
#define Model_HPP_
#include "Math/Matrix.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Model/Common/ModelCommon.hpp"

class Model{
protected:
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

    // world transform
    Microsoft::WRL::ComPtr<ID3D12Resource> wr_;
    Transformation* wd_ = nullptr;

    //Camera
    Microsoft::WRL::ComPtr<ID3D12Resource> cr_;
    CameraForGpu* cd_ = nullptr;

    Camera* camera_ = nullptr;

public:
    Model();
    virtual ~Model() = default;
    virtual void Initialize(const std::string& _name) = 0;
    virtual void Update() = 0;
    virtual void Draw() const = 0;

private:
    virtual void Debug() = 0;
}; // class Model

#endif // Model_HPP_