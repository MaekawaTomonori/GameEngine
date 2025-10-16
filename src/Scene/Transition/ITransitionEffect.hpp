#ifndef ITransitionEffect_HPP_
#define ITransitionEffect_HPP_

/// <summary>
/// トランジションエフェクト基底インターフェース
/// シーン遷移時のエフェクトを実装するための基底クラス
/// </summary>
class ITransitionEffect {
public:
    /// <summary>
    /// トランジション状態
    /// </summary>
    enum class State {
        None,
        In,
        Out,
    };

    virtual ~ITransitionEffect() = default;

    /// <summary>
    /// 初期化処理（純粋仮想関数）
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// 更新処理（純粋仮想関数）
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// 描画処理（純粋仮想関数）
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// トランジションを開始（純粋仮想関数）
    /// </summary>
    /// <param name="_state">トランジション状態</param>
    /// <param name="_duration">継続時間</param>
    virtual void Start(State _state, float _duration) = 0;

    /// <summary>
    /// トランジションを停止（純粋仮想関数）
    /// </summary>
    virtual void Stop() = 0;

    /// <summary>
    /// トランジションが完了したかを判定（純粋仮想関数）
    /// </summary>
    /// <returns>完了した場合true</returns>
    virtual bool IsFinished() const = 0;

}; // class ITransitionEffect

#endif // ITransitionEffect_HPP_
