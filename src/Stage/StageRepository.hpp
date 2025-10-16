#ifndef StageRepository_HPP_
#define StageRepository_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include "LevelData.hpp"

/// <summary>
/// ステージリポジトリクラス
/// レベルデータのキャッシュと管理を提供
/// </summary>
class StageRepository {
    std::unordered_map<std::string, std::unique_ptr<LevelData>> data_;
public:
    /// <summary>
    /// レベルデータを取得
    /// </summary>
    /// <param name="_name">レベル名</param>
    /// <returns>レベルデータポインタ</returns>
    LevelData* Get(const std::string& _name);

    /// <summary>
    /// レベルデータを追加
    /// </summary>
    /// <param name="_name">レベル名</param>
    /// <param name="_data">レベルデータ</param>
    void Add(const std::string& _name, std::unique_ptr<LevelData> _data);

    /// <summary>
    /// レベルデータを削除
    /// </summary>
    /// <param name="_name">レベル名</param>
    void Remove(const std::string& _name);
}; // class StageRepository

#endif // StageRepository_HPP_
