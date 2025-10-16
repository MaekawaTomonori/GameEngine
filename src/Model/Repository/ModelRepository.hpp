#ifndef ModelRepository_HPP_
#define ModelRepository_HPP_
#include <memory>
#include <string>
#include <unordered_map>

#include "src/Model/Data/ModelData.hpp"

/// <summary>
/// モデルリポジトリクラス
/// モデルデータのキャッシュと管理を提供
/// </summary>
class ModelRepository {
    std::unordered_map<std::string, std::unique_ptr<ModelData>> models_;
public:
    void Add(const std::string& _name, std::unique_ptr<ModelData> _model);
    // Nullable
    ModelData* Get(const std::string& _name);
}; // class ModelRepository

#endif // ModelRepository_HPP_
