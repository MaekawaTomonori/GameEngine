#ifndef ModelRepository_HPP_
#define ModelRepository_HPP_
#include <memory>
#include <string>
#include <unordered_map>

#include "WeakPtr.hpp"
#include "src/Model/Data/ModelData.hpp"

/** @brief モデルリポジトリクラス
 * モデルデータのキャッシュと管理を提供
 */
class ModelRepository {
    std::unordered_map<std::string, std::unique_ptr<ModelData>> models_;
public:
    /** @brief モデルデータを追加
     * @param _name モデル名
     * @param _model モデルデータ
     */
    void Add(const std::string& _name, std::unique_ptr<ModelData> _model);

    /** @brief モデルデータを取得（Nullable）
     * @param _name モデル名
     * @return モデルデータポインタ（存在しない場合nullptr）
     */
    GESTD::WeakPtr<ModelData> Get(const std::string& _name);

    bool Contains(const std::string& _name) const;
}; // class ModelRepository

#endif // ModelRepository_HPP_
