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

    // true の場合のみ LightManager::SetPosition の一括反映対象になる
    // （ライトが複数ある場合に全ライトが同じ座標に引っ張られてしまう問題への対処）
    bool followsGlobalRef_ = false;

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

    void SetFollowsGlobalRef(bool _follow) {
        followsGlobalRef_ = _follow;
    }

    bool FollowsGlobalRef() const {
        return followsGlobalRef_;
    }

    std::string GetUUID() const {
        return uuid_;
    }

    /** DefaultAdd
     */
	virtual void DefaultSetting() = 0;
    /** ToJson
     */
    virtual void Save(const std::string& _path) = 0;

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

