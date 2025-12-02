#ifndef StageRepository_HPP_
#define StageRepository_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include "LevelData.hpp"

/** @brief ステージリポジトリクラス
 ** レベルデータのキャッシュと管理を提供
 **/
class StageRepository {
    std::unordered_map<std::string, std::unique_ptr<LevelData>> data_;
public:
    /** @brief レベルデータを取得
     ** @param _name レベル名
     ** @return レベルデータポインタ
     **/
    LevelData* Get(const std::string& _name);

    /** @brief レベルデータを追加
     ** @param _name レベル名
     ** @param _data レベルデータ
     **/
    void Add(const std::string& _name, std::unique_ptr<LevelData> _data);

    /** @brief レベルデータを削除
     ** @param _name レベル名
     **/
    void Remove(const std::string& _name);
}; // class StageRepository

#endif // StageRepository_HPP_
