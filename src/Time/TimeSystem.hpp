#ifndef TimeSystem_HPP_
#define TimeSystem_HPP_

/** @brief ゲーム時間の実データを保持・更新するEngine内部専用クラス
 ** ユーザーには公開しない。Frameworkからの更新(Tick)と、
 ** 公開ハンドルであるTimeからの読み取りにのみ使用する
 **/
class TimeSystem {
public:
    TimeSystem() = default;

    /** @brief ゲームTickの固定間隔(秒)。フレームレートと同じ1/60を基本単位とする **/
    static constexpr float kFixedDeltaTime = 1.f / 60.f;

    /** @brief タイムスケール適用後の、1Tick分の経過秒数を取得
     ** @return 経過秒数
     **/
    float GetDeltaTime() const;

    /** @brief タイムスケール適用前の、1Tick分の経過秒数を取得
     ** @return 経過秒数。常に固定値kFixedDeltaTime
     **/
    float GetUnscaledDeltaTime() const;

    /** @brief 現在のタイムスケールを取得
     ** @return タイムスケール
     **/
    float GetTimeScale() const;

    /** @brief タイムスケールを変更する
     ** @param _scale 適用する倍率
     ** @param _duration この秒数(実時間)が経過すると自動的に1.0fへ戻る。0以下の場合は明示的に戻すまで持続する
     **/
    void SetTimeScale(float _scale, float _duration = 0.f);

    /** @brief 実測の1フレーム分経過秒数を渡して更新する
     ** Frameworkが毎フレーム呼び出す想定。タイムスケールの残り時間(実時間)のカウントダウンにのみ使用する
     ** @param _realDeltaSeconds 実測の経過秒数
     **/
    void Tick(float _realDeltaSeconds);

private:
    float deltaTime_     = 0.f;
    float timeScale_     = 1.f;
    float scaleDuration_ = 0.f; // 残り時間(実時間)。0以下なら無期限
}; // class TimeSystem

#endif // TimeSystem_HPP_
