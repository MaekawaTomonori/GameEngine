#ifndef Time_HPP_
#define Time_HPP_

/** @brief ゲーム全体の時間を管理するクラス
 ** タイムスケール(スローモーション等)を適用した1フレーム分の経過秒数を提供する
 **/
class Time {
public:
    Time() = default;

    /** @brief タイムスケール適用後の、1フレーム分の経過秒数を取得
     ** @return 経過秒数
     **/
    static float GetDeltaTime();

    /** @brief 現在のタイムスケールを取得
     ** @return タイムスケール
     **/
    static float GetTimeScale();

    /** @brief タイムスケールを変更する
     ** @param _scale 適用する倍率
     ** @param _duration この秒数(実時間)が経過すると自動的に1.0fへ戻る。0以下の場合は明示的に戻すまで持続する
     **/
    static void SetTimeScale(float _scale, float _duration = 0.f);

    /** @brief 実測の1フレーム分経過秒数を渡して更新する
     ** Frameworkが毎フレーム呼び出す想定
     ** @param _realDeltaSeconds 実測の経過秒数
     **/
    static void Tick(float _realDeltaSeconds);

private:
    static Time& Instance();

    void TickImpl(float _realDeltaSeconds);
    void SetTimeScaleImpl(float _scale, float _duration);

    float deltaTime_     = 0.f;
    float timeScale_     = 1.f;
    float scaleDuration_ = 0.f; // 残り時間(実時間)。0以下なら無期限
}; // class Time

#endif // Time_HPP_
