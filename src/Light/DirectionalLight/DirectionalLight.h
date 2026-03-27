#pragma once
#include <string>

#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "src/Light/RawLight.h"

struct DirectionalLight{
	Vector4 color;
	Vector3 direction;
    float intensity;
};

class RawDirectionalLight : public RawLight{
    DirectionalLight light_ {};

public:
    ~RawDirectionalLight() override = default;
    DirectionalLight& GetLight() {
        return light_;
    }

	void DefaultSetting() override;
    void Save(const std::string& _path) override;

    void Set(const std::string& _uuid, const DirectionalLight& _light) {
        uuid_ = _uuid;
        light_ = _light;
    }

    void ImGuiSetting(int _index) override;

protected:
    void FollowRef() override;
};