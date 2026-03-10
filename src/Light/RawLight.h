#pragma once
#include <optional>
#include "LightType.hpp"
#include "Utils.hpp"

class RawLight{
protected:
    std::string uuid_;
	LightType type_;
    bool enable_ = true;

    std::optional<Vector3> ref_;

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

    RawLight& SetReference(const Vector3& _ref) {
        ref_ = _ref;
        return *this;
    }

    void ClearRef() {
        ref_.reset();
    }

    std::string GetUUID() const {
        return uuid_;
    }

    /** DefaultAdd
     */
	virtual void DefaultSetting() = 0;
    /** ToJson
     */
    virtual void Save(std::string _path) = 0;

    bool HasRef() const { return ref_.has_value(); }

    virtual void ImGuiSetting(int _index) = 0;

protected:
    virtual void FollowRef() = 0;
};

inline RawLight::RawLight() {
    type_ = LightType::Directional;
    uuid_ = Utils::GenerateUniqueId();
}

inline void RawLight::Update() {
    if (ref_) {
        FollowRef();
    }
}

inline bool RawLight::IsEnable() const {
	return enable_;
}

