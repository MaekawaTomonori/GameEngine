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
    void Initialize(float ratio);
    
    Camera* Add(const std::string& name = "");
    Camera* Get(const std::string& name);
    void Remove(const std::string& name);
    bool Contains(const std::string& name) const;
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