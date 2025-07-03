#pragma once
#include <map>
#include <string>
#include <variant>

#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "vendor/json/json.hpp"

class Json{
    using json = nlohmann::json;
    using Value = std::variant<int32_t, float, Vector2, Vector3, Vector4>;
    using Object = std::map<std::string, Value>; //  key , value || key : [{ key, value }] // Item
    using Group = std::map<std::string, Object>; // group

    const std::string PATH = "Assets/Data/";
    std::map<std::string, Group> datas_; // FileName, Data Groups

    //{
    //  "Data Key": {
    //      "Group" : {
    //              "Object Key" : "Value"
    //     },
    //     "Group" : {
    //              "Object Key" : "Value"
    //      }
    //  }
    //}

    // resume
    // {
    //     "FILE NAME" : {
    //          "KEY" : VALUE
    //     }
    // }

public:
    void SetValue(const std::string& _path, const std::string& _group, const std::string& _key, const Value& _value);
    Group GetGroups(const std::string& _path);
    Value GetValue(const std::string& _path, const std::string& group, const std::string& key) const;
    void RemoveGroup(const std::string& _path, const std::string& _group);

    bool Load(const std::string& _path);
    void Save(const std::string& _path);

private:
    void Register(const std::string& _name);
    void LoadJson(const std::string& _path);
};

