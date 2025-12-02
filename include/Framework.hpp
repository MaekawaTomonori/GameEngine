#ifndef Framework_HPP_
#define Framework_HPP_

#include <memory>

#include "Input.hpp"
#include "DebugUI.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Camera/Director/CameraDirector.hpp"
#include "src/Platform/WinApp.hpp"
#include "src/Config/Config.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/Light/LightManager.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Scene/SceneSwitcher.hpp"
#include "src/Sprite/Common/SpriteCommon.hpp"
#include "src/Line/Common/LineCommon.hpp"
#include "src/ParticleSystem/ParticleSystem.hpp"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"
#include "src/Renderer/Renderer.hpp"
#include "src/Sky/Common/SkyCommon.hpp"
#include "src/Stage/LevelEditor.hpp"
#include "src/Texture/TextureManager.hpp"

class IGame;

/// <summary>
/// メインアプリケーションフレームワーククラス
/// ゲームループの実行とシステムコーディネーションを管理
/// </summary>
class Framework {
    GameEngine::Config config_{};

    std::unique_ptr<IGame> game_;
    SceneSwitcher* scene_ = nullptr;

    std::unique_ptr<WinApp> windows_;
    std::unique_ptr<DirectXAdapter> dxAdapter_;
    std::unique_ptr<PostProcessExecutor> postProcessor_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<ResourceRepository> resources_;
    std::unique_ptr<DebugUI> debugUI_;
    std::unique_ptr<SRVManager> srv_;
    std::unique_ptr<LevelEditor> level_;
    std::unique_ptr<ParticleSystem> particle_;

    Input* input_ = nullptr;
    TextureManager* texture_ = nullptr;
    SpriteCommon* sprite_ = nullptr;
    ModelCommon* model_ = nullptr;
    LineCommon* line_ = nullptr;
    SkyCommon* sky_ = nullptr;
    LightManager* light_ = nullptr;

    CameraController* camera_ = nullptr;
    CameraDirector* cameraDirector_ = nullptr;

public:
    Framework();
    ~Framework();

    /// <summary>
    /// ゲームを実行
    /// </summary>
    /// <param name="_game">実行するゲームインスタンス</param>
    void Execute(std::unique_ptr<IGame> _game);

private:
    /// <summary>
    /// フレームワークの初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// メインループの実行
    /// </summary>
    /// <returns>ループを継続する場合true</returns>
    bool Loop() const;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() const;

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw() const;

    /// <summary>
    /// シャットダウン処理
    /// </summary>
    void Shutdown();

    /// <summary>
    /// システム状態のチェック
    /// </summary>
    /// <returns>正常な場合true</returns>
    bool Check() const;

    /// <summary>
    /// ウィンドウリサイズ処理（DRY原則に基づく抽出）
    /// </summary>
    /// <param name="width">新しい幅</param>
    /// <param name="height">新しい高さ</param>
    void HandleWindowResize(int width, int height) const;
}; // class Framework

#endif // Framework_HPP_
