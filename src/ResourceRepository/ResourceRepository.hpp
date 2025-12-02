#ifndef ResourceRepository_HPP_
#define ResourceRepository_HPP_
#include <memory>

#include "src/Mesh/Repository/MeshRepository.hpp"
#include "src/Model/Repository/ModelRepository.hpp"

/** @brief リソースリポジトリクラス
 ** モデルとメッシュのリポジトリを統合管理
 **/
class ResourceRepository {
    std::unique_ptr<ModelRepository> model_;
    std::unique_ptr<MeshRepository> mesh_;
public:
    /** @brief リソースリポジトリを初期化
     **/
    void Initialize();

    /** @brief モデルリポジトリを取得
     ** @return モデルリポジトリポインタ
     **/
    ModelRepository* GetModelRepository() const;

    /** @brief メッシュリポジトリを取得
     ** @return メッシュリポジトリポインタ
     **/
    MeshRepository* GetMeshRepository() const;
}; // class ResourceRepository

#endif // ResourceRepository_HPP_
