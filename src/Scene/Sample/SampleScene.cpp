#include "SampleScene.hpp"

#include <algorithm>

#include "Input.hpp"
#include "Log.hpp"
#include "Math/Easing.hpp"
#include "src/Time/Time.hpp"
#include "Pattern/Singleton.hpp"
#include "Math/MathUtils.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Screen/Screen.hpp"

#undef min
#undef max

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

    pause_ = GEUI::Canvas::Load("Assets/UI/TestPause.json");
    hud_ = GEUI::Canvas::Load("Assets/UI/TestHud.json");
    resume_ = pause_.Find("Panel/Resume");
    buttons_ = { resume_, pause_.Find("Panel/Retry"), pause_.Find("Panel/Quit") };
    resume_.SetSelected(true);
    quitEnabled_ = true;
    counter_ = GEUI::Canvas::Load("Assets/UI/TestCounter.json");
    counterRoot_ = counter_.Find("Counter");
    digits_ = { counterRoot_.Find("Digit0"), counterRoot_.Find("Digit1"), counterRoot_.Find("Digit2") };
    kills_ = 0;
    introTimer_ = 0.f;
    GEUI::Widget panel = hud_.Find("Panel");
    panel.Find("Overlay").SetColor({0.f, 1.f, 0.f, 0.5f});
    hud_.FindAny("Widget_02").SetVisible(false);
    hud_.Find("Panel/Resum");
}

void SampleScene::Update() {
    model_->Update();
    plane->Update();

    auto i = Singleton<Input>::GetInstance();
    if (kick_ && i->IsTrigger(DIK_SPACE)) {
        i->SetCursorPosition({640.f, 360.f});
    }

    if (i->IsTrigger(DIK_P)) {
        // 実際のポーズ画面と同じく、開くたびに先頭から選ぶ
        pause_.SetVisible(!pause_.IsVisible());
        resume_.SetSelected(true);
    }
    if (i->IsTrigger(DIK_R)) {
        // Button 自身は描かないので、可動部の色を変える
        resume_.Find("Foreground").SetColor({1.f, 0.f, 0.f, 1.f});
    }
    if (i->IsTrigger(DIK_U)) {
        pause_.Unload();
    }

    UpdateButtonSelection();
    UpdateButtonHover();
    UpdateCounter();
    UpdateCubeHover();

    cursorSprite_->SetPosition(i->GetMousePosition());
    cursorSprite_->Update();
}

void SampleScene::UpdateButtonSelection() {
    if (!pause_.IsValid() || !pause_.IsVisible()) return;

    auto i = Singleton<Input>::GetInstance();
    if (i->IsTrigger(DIK_UP)) pause_.Navigate(GEUI::Direction::Up);
    if (i->IsTrigger(DIK_DOWN)) pause_.Navigate(GEUI::Direction::Down);
    if (i->IsTrigger(DIK_RETURN)) pause_.Submit();
    // 取れないスキルと同じく、選べるが押せない状態にする
    if (i->IsTrigger(DIK_E)) {
        quitEnabled_ = !quitEnabled_;
        buttons_[2].SetEnabled(quitEnabled_);
        buttons_[2].Find("Foreground/Label").SetText(quitEnabled_ ? "Quit" : "Quit (Locked)");
    }

    if (resume_.WasPressed()) {
        Log::Send(Log::Level::INFO, "Pause: Resume");
        pause_.SetVisible(false);
    }
    if (buttons_[1].WasPressed()) {
        Log::Send(Log::Level::INFO, "Pause: Retry");
        pause_.SetVisible(false);
    }
    if (buttons_[2].WasPressed()) {
        Log::Send(Log::Level::INFO, "Pause: Quit");
    }
}

void SampleScene::UpdateButtonHover() {
    // 閉じたときに外れたことも見たいので、表示中かどうかでは止めない
    if (!pause_.IsValid()) return;

    constexpr const char* NAMES[] = { "Resume", "Retry", "Quit" };
    for (size_t b = 0; b < buttons_.size(); ++b) {
        const bool hovered = buttons_[b].IsHovered();
        if (hovered != hovered_[b]) {
            Log::Send(Log::Level::INFO, std::string("Hover: ") + NAMES[b] + (hovered ? " on" : " off"));
            hovered_[b] = hovered;
        }
    }
}

void SampleScene::UpdateCounter() {
    auto i = Singleton<Input>::GetInstance();
    if (i->IsTrigger(DIK_K)) {
        kills_ = (kills_ + 1) % 1000;
    }
    if (i->IsTrigger(DIK_T)) {
        introTimer_ = 0.f;
    }

    // numbers.png は 0〜9 が横に 10 個並ぶ
    uint32_t value = kills_;
    for (size_t d = digits_.size(); d-- > 0;) {
        digits_[d].SetImageRegion({static_cast<float>(value % 10) * 0.1f, 0.f}, {0.1f, 1.f});
        value /= 10;
    }

    // 旧 KillCounter と同じく、画面の中央寄りから右上へ滑り込みながら現れる
    constexpr float INTRO_TIME = 0.6f;
    introTimer_ = std::min(introTimer_ + Time().GetDeltaTime(), INTRO_TIME);
    const float t = introTimer_ / INTRO_TIME;
    counterRoot_.SetOffset(Ease::Out::Cubic(Vector2{-0.4f, 0.4f}, Vector2{-0.02f, 0.04f}, t));
    counter_.SetAlpha(t);
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

void SampleScene::Finalize() {
    pause_.Unload();
    hud_.Unload();
    counter_.Unload();
}

void SampleScene::Debug() {
}
