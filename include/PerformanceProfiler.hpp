#ifndef PerformanceProfiler_HPP_
#define PerformanceProfiler_HPP_

#ifdef _DEBUG

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

#include "Pattern/Singleton.hpp"

class DebugUI;

/** @brief CPU タイミング計測デバッガー
 ** PROFILE_SCOPE / PROFILE_BEGIN / PROFILE_END マクロでスコープを計測し
 ** ImGui ウィンドウにセクションごとの最新値と平滑化平均を表示する
 **
 ** インスタンスは Singleton<PerformanceProfiler>::GetInstance() で取得する
 **/
class PerformanceProfiler {
    struct Sample {
        double lastMs   = 0.0;
        double smoothMs = 0.0;  ///< 指数移動平均（係数 0.05）
    };

    std::vector<std::string> order_;
    std::unordered_map<std::string, Sample> samples_;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> active_;

    DebugUI* debugUI_ = nullptr;

public:
    /** @brief 初期化
     ** @param _debug DebugUI へのポインタ
     **/
    void Initialize(DebugUI* _debug);

    /** @brief セクション計測を開始する **/
    void Begin(const char* _name);

    /** @brief セクション計測を終了しサンプルを記録する **/
    void End(const char* _name);

    /** @brief ImGui ウィンドウを登録する **/
    void Debug();

    /** @brief RAII スコープ計測ヘルパー **/
    struct Scope {
        const char* name_;
        explicit Scope(const char* _name) : name_(_name) {
            Singleton<PerformanceProfiler>::GetInstance()->Begin(_name);
        }
        ~Scope() {
            Singleton<PerformanceProfiler>::GetInstance()->End(name_);
        }
    };
};

/** @brief スコープ内の CPU 時間を計測する（ブレースで囲んで使用）
 **   { PROFILE_SCOPE("SceneUpdate"); scene->Update(); }
 **/
#define PROFILE_SCOPE(name) \
    PerformanceProfiler::Scope _prof_scope_(name)

/** @brief 計測を手動で開始する **/
#define PROFILE_BEGIN(name) \
    Singleton<PerformanceProfiler>::GetInstance()->Begin(name)

/** @brief 計測を手動で終了する **/
#define PROFILE_END(name) \
    Singleton<PerformanceProfiler>::GetInstance()->End(name)

#else // !_DEBUG

#define PROFILE_SCOPE(name)  ((void)0)
#define PROFILE_BEGIN(name)  ((void)0)
#define PROFILE_END(name)    ((void)0)

#endif // _DEBUG
#endif // PerformanceProfiler_HPP_
