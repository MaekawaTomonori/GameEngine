#pragma once
#ifdef _DEBUG

#include <string>
#include <string_view>
#include <vector>

#include "imgui.h"
#include "ReferencePtr.hpp"
#include "DebugUI.hpp"
#include "Handle.hpp"
#include "PlaybackHandle.hpp"
#include "TrackHandle.hpp"

/** @brief Audio デバッグパネル
 *  Loader / Player / DAW Mixer の 3 ウィンドウを DebugUI に登録して提供する
 */
class AudioDebugPanel {
public:
    void Initialize(const GESTD::ReferencePtr<DebugUI>& _debugUI);
    void Debug();

private:
    // Data types

    struct LoadedSound {
        std::string       path;
        Audio::Handle     handle;
        int               colorIdx = 0;
    };

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

    struct Track {
        std::string              name;
        Audio::TrackHandle       handle;
        float                    volume   = 1.0f;
        float                    pan      = 0.0f;
        float                    pitch    = 1.0f;
        int                      colorIdx = 0;
        std::vector<PlayingItem> items;
    };

    // Window content renderers
    void DrawLoaderWindow();
    void DrawPlayerWindow();
    void DrawMixerWindow();

    // Mixer strip / list helpers
    void DrawMasterStrip(float groupX);
    void DrawTrackStrip(Track& t, int idx, float groupX, bool& remove);
    void DrawPlaybackList(std::string_view name, ImVec4 headerColor, std::vector<PlayingItem>& items);
    void DrawDualVU(int seed, float volume, bool active, float stripWidth);

    // Track management
    void AddTrack();
    void RemoveTrack(int idx);

    // DebugUI reference 
    GESTD::ReferencePtr<DebugUI> debugUI_;

    // Persistent state 
    char  pathBuf_[256]   = "Assets/Audio/";
    float masterVolume_   = 1.0f;
    int   nextColorIdx_   = 0;
    int   selectedSound_  = -1;
    int   targetTrack_    = -1;
    int   focusedTrack_   = -1;

    float playerVolume_   = 1.0f;
    float playerPitch_    = 1.0f;
    float playerPan_      = 0.0f;
    bool  playerLoop_     = false;
    float listPanelWidth_ = 390.f;

    std::vector<PlayingItem> masterItems_;
    std::vector<LoadedSound> loadedSounds_;
    std::vector<Track>       tracks_;
};

#endif // _DEBUG
