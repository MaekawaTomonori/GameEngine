#ifndef Time_HPP_
#define Time_HPP_

/** @brief ゲーム全体の時間を扱うハンドルクラス
 ** タイムスケール(ヒットストップ、スローモーション等)を適用した
 ** 1Tick分の経過秒数を提供する。
 **/
class Time {
public:
    Time() = default;

    /** @brief タイムスケール適用後の、1Tick分の経過秒数を取得
     ** @return 経過秒数
     **/
    float GetDeltaTime() const;

    /** @brief タイムスケール適用前の、1Tick分の経過秒数を取得
     ** グローバルなタイムスケール(ヒットストップ/スローモーション)に
     ** 関係なく進めたい処理や、独自のローカルスケールを乗算したい場合に使用する
     ** @return 経過秒数
     **/
    float GetUnscaledDeltaTime() const;

    /** @brief 現在のタイムスケールを取得
     ** @return タイムスケール
     **/
    float GetTimeScale() const;

    /** @brief タイムスケールを変更する
     ** ヒットストップやスローモーションなど、演出目的での使用を想定
     ** @param _scale 適用する倍率
     ** @param _duration この秒数(実時間)が経過すると自動的に1.0fへ戻る。0以下の場合は明示的に戻すまで持続する
     **/
    void SetTimeScale(float _scale, float _duration = 0.f);
}; // class Time

#endif // Time_HPP_
