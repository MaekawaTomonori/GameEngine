#ifndef PostProcessPresetEditor_HPP_
#define PostProcessPresetEditor_HPP_

#include <string>
#include <vector>

#include "ReferencePtr.hpp"
#include "json.hpp"

class DebugUI;
class PostProcessExecutor;

/** @brief PostProcess preset editor
 * presets.json の編集と既存 preset の管理を行う。
 * キーフレーム編集にも対応する。
 */
class PostProcessPresetEditor {
public:
    /** @brief preset を構成する effect 定義 */
    struct PresetMember {
        std::string type;        // "Vignette"
        std::string name;        // "MainVignette"
        bool autoCreate = true;  // 自動生成フラグ
    };

    /** @brief preset 編集データ */
    struct PresetData {
        std::string name;                       // "DarkScene"
        std::string mode = "maintain_state";   // "disable_unlisted" or "maintain_state"
        float duration = 2.0f;                 // animation duration
        std::vector<PresetMember> members;     // effect list
        std::vector<std::string> ignoreList;   // ignored effect names
    };

private:
    GESTD::ReferencePtr<DebugUI> debug_ = nullptr;
    GESTD::ReferencePtr<PostProcessExecutor> executor_ = nullptr;

    bool showEditor_ = false;
    bool isEditingPreset_ = false;

    /** Editing state */
    PresetData editingPreset_;
    int selectedMemberIndex_ = -1;

    /** UI state */
    char newPresetNameBuf_[128] = "";
    char newMemberNameBuf_[128] = "Effect1";
    int selectedMemberType_ = 0;
    bool showAddMemberDialog_ = false;

    /** Search and filter state */
    char searchBuffer_[256] = "";
    int sortMode_ = 0; // 0: Name, 1: Duration, 2: Member count

    /** Preview state */
    std::string previewingPresetName_ = "";
    bool isPreviewingPreset_ = false;

    /** Tab state */
    int currentTab_ = 0; // 0: Preset Config, 1: Keyframe Points

    /** Keyframe editing state (simplified) */
    int selectedPointIndex_ = -1;
    nlohmann::json editingKeyframes_;                  // {"Start": {...}, "Middle": {...}, ...}
    std::vector<std::string> editingKeyframeOrder_;   // ["Start", "Middle", "End"]
    bool keyframesDirty_ = false;                     // unsaved changes flag

public:
    PostProcessPresetEditor() = default;
    ~PostProcessPresetEditor() = default;

    /** @brief 初期化 */
    void Initialize(const GESTD::ReferencePtr<DebugUI>& _debug, const GESTD::ReferencePtr<PostProcessExecutor>& _executor);

    /** @brief editor UI を表示 */
    void ShowEditor();

    /** @brief editor を開く */
    void OpenEditor(const std::string& _presetName = "");

    /** @brief editor を閉じる */
    void CloseEditor();

    /** @brief editor が開いているか */
    bool IsOpen() const { return showEditor_; }

private:
    /** @brief preset を読み込んで編集開始 */
    void LoadPresetForEditing(const std::string& _presetName);

    /** @brief 編集中 preset を保存 */
    void SaveEditingPreset();

    /** @brief 新規 preset 作成 */
    void CreateNewPreset(const std::string& _name = "");

    /** @brief preset 編集を終了 */
    void StopEditingPreset();

    /** @brief preset を削除 */
    void DeletePreset(const std::string& _presetName);

    /** @brief 利用可能な preset 一覧を取得 */
    std::vector<std::string> GetAvailablePresets() const;

    /** @brief 利用可能な effect type 一覧を取得 */
    std::vector<std::string> GetAvailableEffectTypes() const;

    /** @brief メンバーを追加 */
    void AddMember(const std::string& _type, const std::string& _name, bool _autoCreate = true);

    /** @brief メンバーを削除 */
    void RemoveMember(int _index);

    /** @brief メンバーを上に移動 */
    void MoveMemberUp(int _index);

    /** @brief メンバーを下に移動 */
    void MoveMemberDown(int _index);

    /** @brief preset を複製 */
    void DuplicatePreset(const std::string& _sourceName, const std::string& _newName);

    /** @brief preset をプレビュー */
    void PreviewPreset(const std::string& _presetName);

    /** @brief プレビューを停止 */
    void StopPreview();

    /** @brief preset を適用 */
    void ApplyPreset(const std::string& _presetName);

    /** @brief preset 名のバリデーション */
    bool ValidatePresetName(const std::string& _name, std::string& _errorMsg) const;

    /** @brief preset 表示用情報 */
    struct PresetInfo {
        std::string name;
        int memberCount;
        float duration;
        std::string mode;
    };
    PresetInfo GetPresetInfo(const std::string& _presetName) const;

    /** @brief フィルタ・ソート済み preset 一覧を取得 */
    std::vector<std::string> GetFilteredAndSortedPresets() const;

    /** UI rendering methods */
    void RenderAvailablePresetsSection();
    void RenderCreateNewPresetSection();
    void RenderPresetConfigurationSection();
    void RenderBasicSettings();
    void RenderMembersList();
    void RenderIgnoreList();
    void RenderAddMemberDialog();
    void RenderQuickSaveSection();

    /** Keyframe points editing methods */
    void RenderKeyframePointsTab();
    void RenderPointsList();
    void RenderPointParameters();

    void LoadMemberKeyframes();
    void SaveMemberKeyframes();
    void PreviewCurrentPoint();

    void AddKeyframePoint(const std::string& _pointName);
    void RemoveKeyframePoint(int _pointIndex);
    void MovePointUp(int _pointIndex);
    void MovePointDown(int _pointIndex);

}; // class PostProcessPresetEditor

#endif // PostProcessPresetEditor_HPP_
