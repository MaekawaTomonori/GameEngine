#ifndef Sprite_HPP_
#define Sprite_HPP_
#include "Math/Matrix.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"
#include "src/DirectX/GraphicsPipeline/GraphicsPipeline.hpp"


class Sprite {
	struct Material {
		Vector4 color;
	};

	struct VertexData {
		Vector4 position;
        Vector2 uv;
	};

    GraphicsPipeline* pipeline_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

    std::string uuid_;

    std::string texturePath_;

    // vertex resource
    Microsoft::WRL::ComPtr<ID3D12Resource> vr_;
    // vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW vbv_{};
    // vertex data
    VertexData* vd_ = nullptr;

    // index resource
    Microsoft::WRL::ComPtr<ID3D12Resource> ir_;
    // index buffer view
    D3D12_INDEX_BUFFER_VIEW ibv_{};
    uint32_t* index_ = nullptr;

    // material resource
    Microsoft::WRL::ComPtr<ID3D12Resource> mr_;
    Material* material_ = nullptr;

    // world transform
    Microsoft::WRL::ComPtr<ID3D12Resource> wr_;
    Matrix4x4* wd_ = nullptr;

    Vector2 position_ = {0, 0};
    Vector2 size_ = {100, 100};

    float rotation_ = 0.f;

    Vector2 anchorPoint_ = {0.5f, 0.5f};
    bool flipX_ = false;
    bool flipY_ = false;

    Vector2 leftTop_{};
    Vector2 texSize_ = {100, 100};


public:
    Sprite(GraphicsPipeline* _pipeline);
    ~Sprite();

    void Initialize();
    void Update();
    void Draw();

    const Vector2& GetPosition() const;

    void SetPosition(const Vector2& p);

    const Vector2& GetSize() const;

    void SetSize(const Vector2& s);

    float GetRotation() const;

    void SetRotation(float r);

    const Vector4& GetColor() const;

    void SetColor(const Vector4& color) const;

    const Vector2& GetAnchorPoint() const;

    void SetAnchorPoint(const Vector2& a);

    bool IsFlipX() const;

    void SetFlipX(bool f);

    bool IsFlipY() const;

    void SetFlipY(bool f);

    const Vector2& GetTextureLeftTop() const;

    void SetTextureLeftTop(const Vector2& textureLeftTop);

    const Vector2& GetTextureSize() const;

    void SetTextureSize(const Vector2& textureSize);

private:
	void AdjustTextureSize();
}; // class Sprite

#endif // Sprite_HPP_
