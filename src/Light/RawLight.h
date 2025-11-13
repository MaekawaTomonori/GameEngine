#pragma once
#include <optional>
#include <functional>
#include "LightType.hpp"
#include "Utils.hpp"

class RawLight{
protected:
    std::string uuid_;
	LightType type_;
    bool enable_ = true;

    std::optional<std::reference_wrapper<Vector3>> ref_;

public:
	RawLight();
	virtual ~RawLight() = default;

    RawLight(const RawLight&) = delete;
    RawLight& operator=(const RawLight&) = delete;

    void Update();

    bool IsEnable() const;

    RawLight& SetUUID(const std::string& uuid) {
        uuid_ = uuid;
        return *this;
    }

    RawLight& SetReference(const std::reference_wrapper<Vector3> _ref) {
        ref_ = _ref;
        return *this;
    }

    std::string GetUUID() const {
        return uuid_;
    }

    //DefaultAdd
	virtual void DefaultSetting() = 0;
    //ToJson
    virtual void Save(std::string _path) = 0;

protected:
    virtual void ImGuiSetting() = 0;
    virtual void FollowRef() = 0;
};

inline RawLight::RawLight() {
    type_ = LightType::Directional;
    uuid_ = Utils::GenerateUniqueId();
}

inline void RawLight::Update() {
#ifdef _DEBUG
    ImGuiSetting();
#endif

    if (ref_) {
        FollowRef();
    }
}

inline bool RawLight::IsEnable() const {
	return enable_;
}

