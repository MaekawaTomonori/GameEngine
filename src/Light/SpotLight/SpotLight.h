#pragma once
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "src/Light/RawLight.h"

struct SpotLight{
	Vector4 color;

    Vector3 position;
    float intensity;

    Vector3 direction;
    float distance;

    float decay;
    float cosAngle;
    float falloffStart;
    float pad;
};

class RawSpotLight : public RawLight{
    SpotLight light_{};

public:
	RawSpotLight() = default;
	RawSpotLight(RawSpotLight&) = delete;
    RawSpotLight& operator=(RawSpotLight&) = delete;

    SpotLight& GetLight() {
        return light_;
    }

    void DefaultSetting() override;
    void Set(const std::string& uuid, const SpotLight& sl);
    void Save(std::string _path) override;

protected:
    void ImGuiSetting() override;
    void FollowRef() override;
};
