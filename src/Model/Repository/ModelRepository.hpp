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
    /// <summary>
    /// モデルデータを追加
    /// </summary>
    /// <param name="_name">モデル名</param>
    /// <param name="_model">モデルデータ</param>
    void Add(const std::string& _name, std::unique_ptr<ModelData> _model);

    /// <summary>
    /// モデルデータを取得（Nullable）
    /// </summary>
    /// <param name="_name">モデル名</param>
    /// <returns>モデルデータポインタ（存在しない場合nullptr）</returns>
    ModelData* Get(const std::string& _name);
}; // class ModelRepository

#endif // ModelRepository_HPP_
