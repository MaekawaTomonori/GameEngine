#ifndef Particle_HPP_
#define Particle_HPP_
#include <string>
#include <vector>
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "src/ParticleSystem/Keyframe.hpp"

class Particle {
    std::string uuid_;

    Vector3 origin_{};
    Vector3 position_{};
    Vector3 scale_{ 1.f, 1.f, 1.f };
    Vector3 velocity_ {};
    Vector3 rotation_{};
    Vector3 rotationVelocity_{};
    Vector4 color_{ 1.f, 1.f, 1.f, 1.f };
    std::vector<GradientKey<Vector4>> colorKeys_;
    std::vector<GradientKey<Vector3>> sizeKeys_;
    float duration_ = 0.f;
    float now_ = 0.f;

public:
    void Initialize(float _duration);

    /** @brief 進行度に応じた色・サイズの補間を行う（フレーム冒頭で全粒子ぶん呼ぶ） */
    void UpdateProgress();

    /** @brief 速度に基づく位置・回転の積分を行う（バッチ更新関数の後で全粒子ぶん呼ぶ） */
    void Integrate();

    void Debug();

    bool IsDead() const;

    float GetProgress() const;
    Vector3 GetOrigin() const;
    Vector3 GetPosition() const;
    Vector3 GetVelocity() const;
    Vector3 GetScale() const;
    Vector3 GetRotation() const;
    Vector4 GetColor() const;

    Particle& SetOrigin(const Vector3& _origin);
    Particle& SetPosition(const Vector3& _position);
    Particle& SetScale(const Vector3& _scale);
    Particle& SetVelocity(const Vector3& _velocity);
    Particle& SetRotation(const Vector3& _rotation);
    Particle& SetRotationVelocity(const Vector3& _rotationVelocity);
    Particle& SetColor(const Vector4& _color);

    /** @brief 色の補間キー（時間昇順にソート済みであること）。空なら色は変化しない */
    Particle& SetColorKeys(const std::vector<GradientKey<Vector4>>& _keys);

    /** @brief サイズの補間キー（時間昇順にソート済みであること）。空ならサイズは変化しない */
    Particle& SetSizeKeys(const std::vector<GradientKey<Vector3>>& _keys);

    Particle& RandomizePosition(const Vector3& _min = { -1.f, -1.f, -1.f }, const Vector3& _max = { 1.f, 1.f, 1.f });
    Particle& RandomizeScale(const Vector3& _min = { 0.1f, 0.1f, 0.1f }, const Vector3& _max = { 1.f, 1.f, 1.f });
    Particle& RandomizeVelocity(const Vector3& _min = { -1.f, -1.f, -1.f }, const Vector3& _max = { 1.f, 1.f, 1.f });
    Particle& RandomizeColor(const Vector4& _min = { 0.f, 0.f, 0.f, 0.f }, const Vector4& _max = { 1.f, 1.f, 1.f, 1.f });

private:

}; // class Particle

#endif // Particle_HPP_
