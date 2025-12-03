#include "Json.hpp"

#include <fstream>

#include "Log.hpp"

void Json::Register(const std::string& _name) {
    if (datas_.contains(_name))return;

    datas_[_name];
}

void Json::LoadJson(const std::string& _path, std::string _name) {
    // Use _path as filename if _name is empty
    if (_name.empty()) {
        _name = _path;
    }

    //Open File
    std::string path = PATH + _path + "/" + _name + ".json";
    std::ifstream file;
    file.open(path);
    if (!file.is_open()){
        Log::Send(Log::Level::ERR, "Failed open file for read");
        assert(false);
        return;
    }

    json root;
    file >> root;
    file.close();

    // Load Data
    auto data = root.find(_path);
    assert(data != root.end());

    for (auto group = data->begin(); group != data->end(); ++group){
        //uuid
        const std::string& groupKey = group.key();

        for (auto object = group->begin(); object != group->end(); ++object){
            const std::string& key = object.key();

            if (object->is_number_integer()){
                int32_t value = object->get<int32_t>();
                SetValue(_path, groupKey, key, value);
            } else if (object->is_number_float()){
                float value = object->get<float>();
                SetValue(_path, groupKey, key,value);
            } else if (object->is_array()){
                if (!object->at(0).is_array()){
                    // Array [v1, v2, ...]
                    if (object->size() == 2){
                        Vector2 value = {object->at(0).get<float>(), object->at(1).get<float>()};
                        SetValue(_path, groupKey, key, value);
                    } else if (object->size() == 3){
                        Vector3 value = {object->at(0).get<float>(), object->at(1).get<float>(), object->at(2).get<float>()};
                        SetValue(_path, groupKey, key, value);
                    } else if (object->size() == 4){
                        Vector4 value = {object->at(0).get<float>(), object->at(1).get<float>(), object->at(2).get<float>(), object->at(3).get<float>()};
                        SetValue(_path, groupKey, key, value);
                    }
                }else {
                    // Array of arrays [[v1,v2,...], [v1,v2,...], ...]
                    auto array = object->at(0);
                    if (array.is_number()) {
                        // Array of floats
                        std::vector<float> floatArray;
                        for (const auto& item : array) {
                            if (item.is_number()) {
                                floatArray.push_back(item.get<float>());
                            }
                        }
                        SetValue(_path, groupKey, key, floatArray);
                    } else if (array.is_array() && array.size() == 2){
                        // Array of Vector2
                        std::vector<Vector2> vectorArray;
                        for (const auto& item : *object) {
                            if (item.is_array() && item.size() == 2) {
                                vectorArray.push_back({ item[0].get<float>(), item[1].get<float>() });
                            }
                        }
                        SetValue(_path, groupKey, key, vectorArray);
                    } else if (array.is_array() && array.size() == 3) {
                        // Array of Vector3
                        std::vector<Vector3> vectorArray;
                        for (const auto& item : *object) {
                            if (item.is_array() && item.size() == 3) {
                                vectorArray.push_back({item[0].get<float>(), item[1].get<float>(), item[2].get<float>()});
                            }
                        }
                        SetValue(_path, groupKey, key, vectorArray);
                    }
                }
            } else if (object->is_string()) {
                std::string value = object->get<std::string>();
                SetValue(_path, groupKey, key, value);
            }
        }
    }
    Log::Send(Log::Level::INFO, "Loaded " + _path + ".json");
}

void Json::SetValue(const std::string& _path, const std::string& _group, const std::string& _key, const Value& _value) {
    Register(_path);

    auto& data = datas_[_path];

    Object& object = data[_group];
    object[_key] = _value;
}

Json::Group Json::GetGroups(const std::string& _path) {
    auto data = datas_.find(_path);
    assert(data != datas_.end());
    return data->second;
}

Json::Value Json::GetValue(const std::string& _path, const std::string& _group, const std::string& _key) const {
    if (!datas_.contains(_path)) return {};
    auto data = datas_.find(_path);
    assert(data != datas_.end());

    auto group = data->second.find(_group);
    assert(group != data->second.end());

    auto item = group->second.find(_key);
    assert(item != group->second.end());

    Value value = item->second;
    return value;
}

void Json::RemoveGroup(const std::string& _path, const std::string& _group) {
    if (!datas_.contains(_path)) return;
    auto data = datas_.find(_path);
    assert(data != datas_.end());
    auto group = data->second.find(_group);
    assert(group != data->second.end());
    data->second.erase(group);
}

bool Json::Load(const std::string& _path, std::string _name) {
    Register(_path);

    Log::Send(Log::Level::INFO, _path + " loading");

    // If _name is specified, load only that file
    if (!_name.empty()) {
        LoadJson(_path, _name);
        return true;
    }

    // If _name is empty, load all JSON files in the directory
    std::filesystem::path dir(PATH + _path + "/");
    if (!exists(dir)){
        return false;
    }

    std::filesystem::directory_iterator itr(dir);
    for (const auto& entry : itr){
        const std::filesystem::path& path = entry.path();

        std::string extension = path.extension().string();
        if (extension != ".json"){
            continue;
        }

        LoadJson(_path, path.stem().string());
    }
    return true;
}

void Json::Save(const std::string& _path, std::string _name) {
    auto group = datas_.find(_path);
    assert(group != datas_.end());

    json root = json::object();
    root[_path] = json::object();

    for (auto& [groupKey, groupData] : group->second){
        root[_path][groupKey] = json::object();
        json& item = root[_path][groupKey];

        for (auto [key, value] : groupData){
            item[key] = json::object();
            if (std::holds_alternative<int32_t>(value)){
                item[key] = std::get<int32_t>(value);
            } else if (std::holds_alternative<float>(value)){
                item[key] = std::get<float>(value);
            } else if (std::holds_alternative<Vector2>(value)){
                Vector2 v = std::get<Vector2>(value);
                item[key] = { v.x, v.y };
            } else if (std::holds_alternative<Vector3>(value)){
                Vector3 v = std::get<Vector3>(value);
                item[key] = { v.x, v.y, v.z };
            } else if (std::holds_alternative<Vector4>(value)){
                Vector4 v = std::get<Vector4>(value);
                item[key] = { v.x, v.y, v.z, v.w };
            } else if (std::holds_alternative<std::vector<float>>(value)){
                std::vector<float> v = std::get<std::vector<float>>(value);
                item[key] = v;
            } else if (std::holds_alternative<std::vector<Vector2>>(value)){
                json array = json::array();
                for (const auto& [x, y] : std::get<std::vector<Vector2>>(value)){
                    array.push_back({ x, y});
                }
                item[key] = array;
            } else if (std::holds_alternative<std::vector<Vector3>>(value)){
                json array = json::array();
                for (const auto& [x, y, z] : std::get<std::vector<Vector3>>(value)) {
                    array.push_back({x, y, z});
                }
                item[key] = array;
            } else if (std::holds_alternative<std::string>(value)){
                item[key] = std::get<std::string>(value);
            }
        }
    }

    std::filesystem::path dir(PATH + _path + "/");
    if (!exists(dir)){
        create_directories(dir);
    }

    if (_name.empty()) _name = _path;

    std::string path = dir.string() + _name + ".json";
    std::ofstream file(path, std::ios::trunc);

    if (!file.is_open()){
        Log::Send(Log::Level::ERR, "Failed open file for write");
        return;
    }

    file << root.dump(4) << '\n';
    file.close();

    datas_.erase(_path);

    Log::Send(Log::Level::INFO, "Saved " + _path);
}
