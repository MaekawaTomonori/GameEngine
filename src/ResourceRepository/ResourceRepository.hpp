#ifndef ResourceRepository_HPP_
#define ResourceRepository_HPP_
#include <memory>

#include "src/Mesh/Repository/MeshRepository.hpp"
#include "src/Model/Repository/ModelRepository.hpp"

/// <summary>
/// リソースリポジトリクラス
/// モデルとメッシュのリポジトリを統合管理
/// </summary>
class ResourceRepository {
    std::unique_ptr<ModelRepository> model_;
    std::unique_ptr<MeshRepository> mesh_;
public:
    /// <summary>
    /// リソースリポジトリを初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// モデルリポジトリを取得
    /// </summary>
    /// <returns>モデルリポジトリポインタ</returns>
    ModelRepository* GetModelRepository() const;

    /// <summary>
    /// メッシュリポジトリを取得
    /// </summary>
    /// <returns>メッシュリポジトリポインタ</returns>
    MeshRepository* GetMeshRepository() const;
}; // class ResourceRepository

#endif // ResourceRepository_HPP_
