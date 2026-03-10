#ifndef Framework_HPP_
#define Framework_HPP_

#include <memory>

#include "Input.hpp"
#ifdef _DEBUG
#include "src/Debug/Debugger.hpp"
#endif
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
#include "src/Collision/CollisionManager.hpp"
#include "src/PostProcess/Executor/PostProcessExecutor.hpp"
#include "src/Renderer/Renderer.hpp"
#include "src/Sky/Common/SkyCommon.hpp"
#include "src/Stage/LevelEditor.hpp"
#include "src/Texture/TextureManager.hpp"

class IGame;

/** @brief メインアプリケーションフレームワーククラス
 * ゲームループの実行とシステムコーディネーションを管理
 */
class Framework {
    GameEngine::Config config_{};

    std::unique_ptr<IGame> game_;
    SceneSwitcher* scene_ = nullptr;

    std::unique_ptr<WinApp> windows_;
    std::unique_ptr<DirectXAdapter> dxAdapter_;
    std::unique_ptr<CollisionManager> collision_;
    std::unique_ptr<PostProcessExecutor> postProcessor_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<ResourceRepository> resources_;
#ifdef _DEBUG
    std::unique_ptr<Debugger> debugger_;
#endif
    std::unique_ptr<SRVManager> srv_;
    //std::unique_ptr<LevelEditor> level_;
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

    /** @brief ゲームを実行
     * @param _game 実行するゲームインスタンス
     */
    void Execute(std::unique_ptr<IGame> _game);

private:
    /** @brief フレームワークの初期化
     */
    void Initialize();

    /** @brief メインループの実行
     * @return ループを継続する場合true
     */
    bool Loop() const;

    /** @brief 更新処理
     */
    void Update() const;

    /** @brief 描画処理
     */
    void Draw() const;

    /** @brief シャットダウン処理
     */
    void Shutdown();

    /** @brief システム状態のチェック
     * @return 正常な場合true
     */
    bool Check() const;

    /** @brief ウィンドウリサイズ処理（DRY原則に基づく抽出）
     * @param _width 新しい幅
     * @param _height 新しい高さ
     */
    void HandleWindowResize(int _width, int _height) const;
}; // class Framework

#endif // Framework_HPP_
