#ifndef ITransitionEffect_HPP_
#define ITransitionEffect_HPP_

/** @brief トランジションエフェクト基底インターフェース
 ** シーン遷移時のエフェクトを実装するための基底クラス
 **/
class ITransitionEffect {
public:
    /** @brief トランジション状態
     **/
    enum class State {
        None,
        In,
        Out,
    };

    virtual ~ITransitionEffect() = default;

    /** @brief 初期化処理（純粋仮想関数）
     **/
    virtual void Initialize() = 0;

    /** @brief 更新処理（純粋仮想関数）
     **/
    virtual void Update() = 0;

    /** @brief 描画処理（純粋仮想関数）
     **/
    virtual void Draw() = 0;

    /** @brief トランジションを開始（純粋仮想関数）
     ** @param _state トランジション状態
     ** @param _duration 継続時間
     **/
    virtual void Start(State _state, float _duration) = 0;

    /** @brief トランジションを停止（純粋仮想関数）
     **/
    virtual void Stop() = 0;

    /** @brief トランジションが完了したかを判定（純粋仮想関数）
     ** @return 完了した場合true
     **/
    virtual bool IsFinished() const = 0;

}; // class ITransitionEffect

#endif // ITransitionEffect_HPP_
