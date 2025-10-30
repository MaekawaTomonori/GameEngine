#ifndef PostProcessPresetEditor_HPP_
#define PostProcessPresetEditor_HPP_

#include <string>
#include <vector>
#include "vendor/json/json.hpp"

class DebugUI;
class PostProcessExecutor;

/// <summary>
/// PostProcessプリセット構成エディター
/// presets.jsonの編集のみを担当（プリセット構成管理）
/// キーフレーム編集は各Effectが担当
/// </summary>
class PostProcessPresetEditor {
public:
    /// <summary>
    /// プリセットメンバー（エフェクト構成）
    /// </summary>
    struct PresetMember {
        std::string type;        // "Vignette"
        std::string name;        // "MainVignette"
        bool autoCreate = true;  // 自動生成フラグ
    };

    /// <summary>
    /// プリセット構成データ
    /// </summary>
    struct PresetData {
        std::string name;                       // "DarkScene"
        std::string mode = "maintain_state";    // "disable_unlisted" or "maintain_state"
        float duration = 2.0f;                  // アニメーション時間
        std::vector<PresetMember> members;      // エフェクトリスト
        std::vector<std::string> ignoreList;    // 無視リスト
    };

private:
    DebugUI* debug_ = nullptr;
    PostProcessExecutor* executor_ = nullptr;

    bool showEditor_ = false;
    bool isEditingPreset_ = false;

    // Editing state
    PresetData editingPreset_;
    int selectedMemberIndex_ = -1;

    // UI state
    char newPresetNameBuf_[128] = "";
    char newMemberNameBuf_[128] = "Effect1";
    int selectedMemberType_ = 0;
    bool showAddMemberDialog_ = false;

    // Search and filter state
    char searchBuffer_[256] = "";
    int sortMode_ = 0; // 0: Name, 1: Duration, 2: Member count

    // Preview state
    std::string previewingPresetName_ = "";
    bool isPreviewingPreset_ = false;

    // Tab state
    int currentTab_ = 0; // 0: Preset Config, 1: Keyframe Points

    // Keyframe editing state (simplified)
    int selectedPointIndex_ = -1;
    nlohmann::json editingKeyframes_;  // {"Start": {...params...}, "Middle": {...}, ...}
    std::vector<std::string> editingKeyframeOrder_;  // ["Start", "Middle", "End"]
    bool keyframesDirty_ = false;  // 未保存の変更フラグ

public:
    PostProcessPresetEditor() = default;
    ~PostProcessPresetEditor() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(DebugUI* _debug, PostProcessExecutor* _executor);

    /// <summary>
    /// エディターウィンドウを表示
    /// </summary>
    void ShowEditor();

    /// <summary>
    /// エディターを開く
    /// </summary>
    void OpenEditor(const std::string& _presetName = "");

    /// <summary>
    /// エディターを閉じる
    /// </summary>
    void CloseEditor();

    /// <summary>
    /// エディターが開いているか
    /// </summary>
    bool IsOpen() const { return showEditor_; }

private:
    /// <summary>
    /// プリセットを読み込んで編集開始
    /// </summary>
    void LoadPresetForEditing(const std::string& _presetName);

    /// <summary>
    /// 編集中のプリセットを保存
    /// </summary>
    void SaveEditingPreset();

    /// <summary>
    /// 新規プリセット作成
    /// </summary>
    void CreateNewPreset(const std::string& _name = "");

    /// <summary>
    /// プリセット編集を停止
    /// </summary>
    void StopEditingPreset();

    /// <summary>
    /// プリセットを削除
    /// </summary>
    void DeletePreset(const std::string& _presetName);

    /// <summary>
    /// 利用可能なプリセット一覧を取得
    /// </summary>
    std::vector<std::string> GetAvailablePresets() const;

    /// <summary>
    /// 利用可能なエフェクトタイプ一覧を取得
    /// </summary>
    std::vector<std::string> GetAvailableEffectTypes() const;

    /// <summary>
    /// メンバー追加
    /// </summary>
    void AddMember(const std::string& _type, const std::string& _name, bool _autoCreate = true);

    /// <summary>
    /// メンバー削除
    /// </summary>
    void RemoveMember(int index);

    /// <summary>
    /// メンバーを上に移動
    /// </summary>
    void MoveMemberUp(int index);

    /// <summary>
    /// メンバーを下に移動
    /// </summary>
    void MoveMemberDown(int index);

    /// <summary>
    /// プリセットを複製
    /// </summary>
    void DuplicatePreset(const std::string& _sourceName, const std::string& _newName);

    /// <summary>
    /// プリセットをプレビュー
    /// </summary>
    void PreviewPreset(const std::string& _presetName);

    /// <summary>
    /// プレビューを停止
    /// </summary>
    void StopPreview();

    /// <summary>
    /// プリセットを実行（Executorに適用）
    /// </summary>
    void ApplyPreset(const std::string& _presetName);

    /// <summary>
    /// プリセット名のバリデーション
    /// </summary>
    bool ValidatePresetName(const std::string& _name, std::string& _errorMsg) const;

    /// <summary>
    /// プリセット情報を取得（表示用）
    /// </summary>
    struct PresetInfo {
        std::string name;
        int memberCount;
        float duration;
        std::string mode;
    };
    PresetInfo GetPresetInfo(const std::string& _presetName) const;

    /// <summary>
    /// フィルタ・ソート済みプリセット一覧を取得
    /// </summary>
    std::vector<std::string> GetFilteredAndSortedPresets() const;

    // UI Rendering Methods
    void RenderAvailablePresetsSection();
    void RenderCreateNewPresetSection();
    void RenderPresetConfigurationSection();
    void RenderBasicSettings();
    void RenderMembersList();
    void RenderIgnoreList();
    void RenderAddMemberDialog();
    void RenderQuickSaveSection();

    // Keyframe Points Editing Methods (redesigned)
    void RenderKeyframePointsTab();
    void RenderPointsList();
    void RenderPointParameters();  // 現在選択中のポイントのパラメータを編集

    void LoadMemberKeyframes();   // JSON → editingKeyframes_ (一括読み込み)
    void SaveMemberKeyframes();   // editingKeyframes_ → JSON (一括保存)
    void PreviewCurrentPoint();   // 現在のポイントをエフェクトに適用

    void AddKeyframePoint(const std::string& pointName);
    void RemoveKeyframePoint(int pointIndex);
    void MovePointUp(int pointIndex);
    void MovePointDown(int pointIndex);

}; // class PostProcessPresetEditor

#endif // PostProcessPresetEditor_HPP_