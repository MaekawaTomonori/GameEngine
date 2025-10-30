#ifndef PostProcessExecutor_HPP_
#define PostProcessExecutor_HPP_
#include <vector>
#include <memory>
#include <functional>
#include <d3d12.h>

#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"
#include "src/PostProcess/Editor/PostProcessPresetEditor.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class DirectXAdapter;
class Heap;
class AbstractPostEffectFactory;

class PostProcessExecutor {
    struct EffectData {
        std::unique_ptr<IPostEffect> effect;
        std::string name;
        std::string type;
        bool enabled = true;
    };

    struct Work {
        struct Member {
            std::string name;
            std::string type;
            bool autoCreate;
        };

        std::string name;
        float duration;
        std::vector<Member> members;
        std::vector<std::string> ignore;
    };

    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;
    DebugUI* debugUI_ = nullptr;
    AbstractPostEffectFactory* factory_ = nullptr;  // PostEffectファクトリー

    std::vector<EffectData> effects_;

    // シーン描画用RenderTexture
    std::unique_ptr<DX12Resource> renderTexture_;
    std::unique_ptr<Heap> rtvHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_{};

    std::unique_ptr<PipelineStateObject> pso_;

    Vector4 clearColor_{ 0.2f, 0.2f, 0.2f, 1.0f };
    uint32_t srvIndex_ = 0;

    std::unique_ptr<DX12Resource> tempBuffer_;

    std::vector<Work> works_;
    float currentTimer_ = 0.f;
    float duration_ = 0.f;
    std::string currentWork_ {};

    // Animation state
    bool isAnimating_ = false;
    float animationTimer_ = 0.f;
    float animationDuration_ = 0.f;
    std::vector<std::string> animatingEffects_;  // エフェクト名リスト
    std::function<void()> onAnimationComplete_;  // アニメーション完了コールバック

    // Preset editor
    std::unique_ptr<PostProcessPresetEditor> presetEditor_;

public:
    /// <summary>
    /// 初期化処理
    /// <param name=_adapter"> DirectXアダプター</param>
    ///  <param name=_srv"> SRVマネージャー</param>
    /// <param name=_debug"> デバッグUI</param>
    /// </summary>
    void Initialize(DirectXAdapter* _adapter, SRVManager* _srv, DebugUI* _debug);

    /// <summary>
    /// ポストエフェクトファクトリーの設定
    /// </summary>
    void SetFactory(AbstractPostEffectFactory* _factory);

    /// <summary>
    ///  エフェクトの追加
    ///
    void Add(std::unique_ptr<IPostEffect> _effect, const std::string& _name = "NoName");

    void BeginFrame() const;
    void EndFrame() const;
    void Execute();
    void Draw() const;

    void SetActive(const std::string& _name, bool _enable);

    void Debug();

    /// <summary>
    /// プリセットを適用（遅延初期化 + アニメーション開始）
    /// </summary>
    /// <param name="_presetName">プリセット名（presets.jsonのキー）</param>
    /// <param name="_mode">"add"または"replace"</param>
    /// <param name="_ignoreList">無視するエフェクト名リスト</param>
    /// <param name="_onComplete">アニメーション完了時のコールバック（オプション）</param>
    void ApplyPreset(
        const std::string& _presetName,
        const std::string& _mode = "replace",
        const std::vector<std::string>& _ignoreList = {},
        std::function<void()> _onComplete = nullptr
    );

    /// <summary>
    /// アニメーション更新（毎フレーム呼び出し）
    /// </summary>
    /// <param name="_deltaTime">フレーム間時間</param>
    void Update(float _deltaTime);

    /// <summary>
    /// 現在のアクティブエフェクトの状態をJSONに保存
    /// </summary>
    /// <param name="_presetName">保存先プリセット名</param>
    void SavePreset(const std::string& _presetName);

    /// <summary>
    /// プリセットエディターを開く
    /// </summary>
    /// <param name="_presetName">編集するプリセット名（空なら新規作成）</param>
    void OpenPresetEditor(const std::string& _presetName = "");

    /// @brief エフェクトを検索、なければ生成
    /// @param _type エフェクトタイプ名
    /// @param _name エフェクトインスタンス名
    /// @param _create インスタンスが存在しない場合に生成するか
    /// @return エフェクトポインタ、生成失敗時はnullptr
    IPostEffect* FindOrCreate(const std::string& _type, const std::string& _name, bool _create);

private:
    void CreateSceneRenderTexture();
}; // class PostProcessExecutor

#endif // PostProcessExecutor_HPP_
