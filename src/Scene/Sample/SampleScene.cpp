#include "SampleScene.hpp"

#include "Input.hpp"
#include "Pattern/Singleton.hpp"
#include "Math/MathUtils.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Screen/Screen.hpp"

void SampleScene::Initialize() {
    model_ = std::make_unique<Model>();
    model_->Initialize("animatedCube");
    model_->SetTranslate(cubePosition_);
    model_->SetScale({0.4f, 0.4f, 0.4f});

    plane = std::make_unique<Model>();
    plane->Initialize("plane");
    plane->SetTranslate({0.f, -0.4f, 0.f});
    plane->SetRotate({4.5f, 0.f, 0.f});
    plane->SetScale({5.f, 5.f, 1.f});

    kick_.emplace("Assets/audio/kick.wav");

    // フルスクリーン時の座標変換デバッグ用：マウス追従カーソルを表示
    Singleton<Input>::GetInstance()->SetCursorVisible(true);

    cursorSprite_ = std::make_unique<Sprite>();
    cursorSprite_->Initialize("circle.png");
    cursorSprite_->SetSize({32.f, 32.f});
    cursorSprite_->SetAnchorPoint({0.5f, 0.5f});
}

void SampleScene::Update() {
    model_->Update();
    plane->Update();

    auto i = Singleton<Input>::GetInstance();
    if (kick_ && i->IsTrigger(DIK_SPACE)) {
        i->SetCursorPosition({640.f, 360.f});
    }

    UpdateCubeHover();

    cursorSprite_->SetPosition(i->GetMousePosition());
    cursorSprite_->Update();
}

void SampleScene::UpdateCubeHover() {
    const auto camera = Singleton<CameraController>::GetInstance()->GetActive();
    if (!camera) return;

    // Cubeのワールド座標をスクリーン座標（ライブ解像度空間）へ射影する
    const Matrix4x4 viewProjection = camera->GetViewProjection();
    const Vector3   ndc            = MathUtils::Matrix::Transform(cubePosition_, viewProjection);

    const float liveWidth  = Singleton<Screen>::GetInstance()->Width();
    const float liveHeight = Singleton<Screen>::GetInstance()->Height();

    const Vector2 screenPos = {
        (ndc.x * 0.5f + 0.5f) * liveWidth,
        (1.f - (ndc.y * 0.5f + 0.5f)) * liveHeight
    };

    const Vector2 mouse = Singleton<Input>::GetInstance()->GetMousePosition();
    const Vector2 diff  = { mouse.x - screenPos.x, mouse.y - screenPos.y };
    const float   distSq = diff.x * diff.x + diff.y * diff.y;

    constexpr float kHoverRadius = 60.f;
    const bool hovered = distSq <= kHoverRadius * kHoverRadius;

    model_->SetColor(hovered ? Vector4{1.f, 0.f, 0.f, 1.f} : Vector4{1.f, 1.f, 1.f, 1.f});
}

void SampleScene::Draw() {
    model_->Draw();
    plane->Draw();
    cursorSprite_->Draw();
}

void SampleScene::Debug() {
}
