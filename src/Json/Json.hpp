#ifndef JSON_HPP_
#define JSON_HPP_

#include <map>
#include <string>
#include <variant>

#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "vendor/json/json.hpp"

class Json{
    using json = nlohmann::json;
    using Value = std::variant<int32_t, float, Vector2, Vector3, Vector4,std::vector<float>, std::vector<Vector2>, std::vector<Vector3>, std::string>;
    using Object = std::map<std::string, Value>; //  key , value || key : [{ key, value }] // Item
    using Group = std::map<std::string, Object>; // group

    const std::string PATH = "Assets/Data/";
    std::map<std::string, Group> datas_; // Data Key, Data Groups

    // JSON File Structure:
    // {
    //     "Data Key": {
    //         "Group1": {
    //             "key1": value1,
    //             "key2": value2
    //         },
    //         "Group2": {
    //             "key3": value3
    //         }
    //     }
    // }
    //
    // Usage:
    // Load("directory", "filename") -> datas_["filename"] stores data from "Assets/Data/directory/filename.json"
    // Load("directory") -> loads all JSON files in directory, each stored with its filename as key

public:
    void SetValue(const std::string& _name, const std::string& _group, const std::string& _key, const Value& _value);
    Group GetGroups(const std::string& _name);
    Value GetValue(const std::string& _name, const std::string& _group, const std::string& _key) const;
    void RemoveGroup(const std::string& _name, const std::string& _group);

    bool Load(const std::string& _path, const std::string& _name = "");
    void Save(const std::string& _path, std::string _name = "");

private:
    void Register(const std::string& _name);
    void LoadJson(const std::string& _path, std::string _name);
};

#endif // JSON_HPP_
