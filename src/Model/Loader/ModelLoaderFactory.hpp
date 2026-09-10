#ifndef ModelLoaderFactory_HPP_
#define ModelLoaderFactory_HPP_
#include <string>

#include "ReferencePtr.hpp"

class ResourceRepository;

/** @brief モデルアセットの読み込みを担う内部ファクトリ
 * 拡張子からローダー（ObjLoader/GltfLoader）を選択して実行する
 */
class ModelLoaderFactory {
public:
    /** @brief モデルを読み込み、リポジトリへ登録する
     * @param _name モデル名
     * @param _repository 登録先のリソースリポジトリ
     */
    static void Load(const std::string& _name, const GESTD::ReferencePtr<ResourceRepository>& _repository);
}; // class ModelLoaderFactory

#endif // ModelLoaderFactory_HPP_
