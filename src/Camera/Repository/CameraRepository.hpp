#ifndef CameraRepository_HPP_
#define CameraRepository_HPP_
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/Camera/Camera.hpp"

class CameraRepository {
    std::unordered_map<std::string, std::unique_ptr<Camera>> cameras_;
    uint16_t nonameCount_ = 0;
    float ratio_ = 0.0f;

public:
    void Initialize(float _ratio);

    Camera* Add(const std::string& _name = "");
    Camera* Get(const std::string& _name);
    void Remove(const std::string& _name);
    bool Contains(const std::string& _name) const;
    bool IsEmpty() const;

    std::vector<std::string> GetNames() const;
    std::string GetFirstName() const;

    void LoadFromFile();
    void SaveToFile();
    void Clear();

private:
    std::string GenerateUniqueName();
}; // class CameraRepository

#endif // CameraRepository_HPP_
