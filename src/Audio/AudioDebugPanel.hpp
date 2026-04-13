#pragma once
#ifdef _DEBUG

#include <string>
#include <vector>

#include "imgui.h"

#include "AudioLib.hpp"

class DebugUI;

/** @brief オーディオデバッグパネル（Debugビルド専用）
 * AudioLib の再生・ミキサー操作を ImGui で行う DAW 風デバッグ UI
 * DebugUI へ "Audio Loader" / "Audio Player" / "Audio Mixer" の 3 ウィンドウを登録する
 */
class AudioDebugPanel {

    /** @brief ロード済みサウンドアセット */
    struct LoadedSound {
        std::string   path;
        Audio::Handle handle;
        int           colorIdx = 0;
    };

    /** @brief 1 つの再生インスタンス */
    struct PlayingItem {
        std::string           soundName;
        Audio::PlaybackHandle playback;
        float volume = 1.0f;
        float pitch  = 1.0f;
        float pan    = 0.0f;
        bool  muted  = false;
        bool  loop   = false;
        bool  paused = false;
    };

    /** @brief ミキサートラック（Audio::TrackHandle に対応） */
    struct Track {
        std::string        name;
        Audio::TrackHandle handle;
        float volume   = 1.0f;
        float pan      = 0.0f;
        float pitch    = 1.0f;
        int   colorIdx = 0;
        std::vector<PlayingItem> items;
    };

    DebugUI* debugUI_ = nullptr;

    char  pathBuf_[256]  = "Assets/Audio/";
    float masterVolume_  = 1.0f;
    int   nextColorIdx_  = 0;
    int   selectedSound_ = -1;
    int   targetTrack_   = -1;
    int   focusedTrack_  = -1;

    float playerVolume_   = 1.0f;
    float playerPitch_    = 1.0f;
    float playerPan_      = 0.0f;
    bool  playerLoop_     = false;
    float listPanelWidth_ = 360.f;

    std::vector<PlayingItem> masterItems_;
    std::vector<LoadedSound> loadedSounds_;
    std::vector<Track>       tracks_;

public:

    /** @brief 初期化 - DebugUI にウィンドウを登録する
     * @param _debugUI DebugUI ポインタ
     */
    void Initialize(DebugUI* _debugUI);

    /** @brief 毎フレーム呼び出す - DebugUI へコマンドを登録する
     */
    void Debug();

private:
    void DrawLoaderPanel();
    void DrawPlayerPanel();
    void DrawMixerPanel();

    void DrawLoaderContent();
    void DrawPlayerContent();
    void DrawMixerContent();

    void DrawMasterStrip(float groupX);
    void DrawTrackStrip(Track& t, int idx, float groupX, bool& remove);
    void DrawPlaybackList(std::string_view name, ImVec4 headerColor,
                          std::vector<PlayingItem>& items);
    void DrawDualVU(int seed, float volume, bool active, float stripWidth);

    void AddTrack();
    void RemoveTrack(int idx);
};

#endif // _DEBUG
