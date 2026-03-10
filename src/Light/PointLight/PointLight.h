#pragma once
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "src/Light/RawLight.h"

struct PointLight{
	Vector4 color;

	Vector3 position;
	float intensity;

	float radius;
	float decay;
    float pad[2];
};

class RawPointLight final : public RawLight{
	PointLight light_ {};
public:
    PointLight& GetLight() {
        return light_;
    }

	void DefaultSetting() override;
	void Set(const std::string& uuid, const PointLight& pl);
	void Save(std::string _path) override;
	void ImGuiSetting(int _index) override;

protected:
    void FollowRef() override;
};

