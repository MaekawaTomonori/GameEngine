#ifndef PostProcessPresetEditor_HPP_
#define PostProcessPresetEditor_HPP_

#include <string>
#include <vector>
#include "json.hpp"

class DebugUI;
class PostProcessExecutor;

/** @brief PostProcessプリセット構成エディター
 * presets.jsonの編集のみを担当（プリセット構成管理）
 * キーフレーム編集は各Effectが担当
 */
class PostProcessPresetEditor {
public:
    /** @brief プリセットメンバー（エフェクト構成）
     */
    struct PresetMember {
        std::string type;        // "Vignette"
        std::string name;        // "MainVignette"
        bool autoCreate = true;  // 自動生成フラグ
    };

    /** @brief プリセット構成データ
     */
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

    /** Editing state
     */
    PresetData editingPreset_;
    int selectedMemberIndex_ = -1;

    /** UI state
     */
    char newPresetNameBuf_[128] = "";
    char newMemberNameBuf_[128] = "Effect1";
    int selectedMemberType_ = 0;
    bool showAddMemberDialog_ = false;

    /** Search and filter state
     */
    char searchBuffer_[256] = "";
    int sortMode_ = 0; // 0: Name, 1: Duration, 2: Member count

    /** Preview state
     */
    std::string previewingPresetName_ = "";
    bool isPreviewingPreset_ = false;

    /** Tab state
     */
    int currentTab_ = 0; // 0: Preset Config, 1: Keyframe Points

    /** Keyframe editing state (simplified)
     */
    int selectedPointIndex_ = -1;
    nlohmann::json editingKeyframes_;  // {"Start": {...params...}, "Middle": {...}, ...}
    std::vector<std::string> editingKeyframeOrder_;  // ["Start", "Middle", "End"]
    bool keyframesDirty_ = false;  // 未保存の変更フラグ

public:
    PostProcessPresetEditor() = default;
    ~PostProcessPresetEditor() = default;

    /** @brief 初期化
     */
    void Initialize(DebugUI* _debug, PostProcessExecutor* _executor);

    /** @brief エディターウィンドウを表示
     */
    void ShowEditor();

    /** @brief エディターを開く
     */
    void OpenEditor(const std::string& _presetName = "");

    /** @brief エディターを閉じる
     */
    void CloseEditor();

    /** @brief エディターが開いているか
     */
    bool IsOpen() const { return showEditor_; }

private:
    /** @brief プリセットを読み込んで編集開始
     */
    void LoadPresetForEditing(const std::string& _presetName);

    /** @brief 編集中のプリセットを保存
     */
    void SaveEditingPreset();

    /** @brief 新規プリセット作成
     */
    void CreateNewPreset(const std::string& _name = "");

    /** @brief プリセット編集を停止
     */
    void StopEditingPreset();

    /** @brief プリセットを削除
     */
    void DeletePreset(const std::string& _presetName);

    /** @brief 利用可能なプリセット一覧を取得
     */
    std::vector<std::string> GetAvailablePresets() const;

    /** @brief 利用可能なエフェクトタイプ一覧を取得
     */
    std::vector<std::string> GetAvailableEffectTypes() const;

    /** @brief メンバー追加
     */
    void AddMember(const std::string& _type, const std::string& _name, bool _autoCreate = true);

    /** @brief メンバー削除
     */
    void RemoveMember(int _index);

    /** @brief メンバーを上に移動
     */
    void MoveMemberUp(int _index);

    /** @brief メンバーを下に移動
     */
    void MoveMemberDown(int _index);

    /** @brief プリセットを複製
     */
    void DuplicatePreset(const std::string& _sourceName, const std::string& _newName);

    /** @brief プリセットをプレビュー
     */
    void PreviewPreset(const std::string& _presetName);

    /** @brief プレビューを停止
     */
    void StopPreview();

    /** @brief プリセットを実行（Executorに適用）
     */
    void ApplyPreset(const std::string& _presetName);

    /** @brief プリセット名のバリデーション
     */
    bool ValidatePresetName(const std::string& _name, std::string& _errorMsg) const;

    /** @brief プリセット情報を取得（表示用）
     */
    struct PresetInfo {
        std::string name;
        int memberCount;
        float duration;
        std::string mode;
    };
    PresetInfo GetPresetInfo(const std::string& _presetName) const;

    /** @brief フィルタ・ソート済みプリセット一覧を取得
     */
    std::vector<std::string> GetFilteredAndSortedPresets() const;

    /** UI Rendering Methods
     */
    void RenderAvailablePresetsSection();
    void RenderCreateNewPresetSection();
    void RenderPresetConfigurationSection();
    void RenderBasicSettings();
    void RenderMembersList();
    void RenderIgnoreList();
    void RenderAddMemberDialog();
    void RenderQuickSaveSection();

    /** Keyframe Points Editing Methods (redesigned)
     */
    void RenderKeyframePointsTab();
    void RenderPointsList();
    void RenderPointParameters();  // 現在選択中のポイントのパラメータを編集

    void LoadMemberKeyframes();   // JSON → editingKeyframes_ (一括読み込み)
    void SaveMemberKeyframes();   // editingKeyframes_ → JSON (一括保存)
    void PreviewCurrentPoint();   // 現在のポイントをエフェクトに適用

    void AddKeyframePoint(const std::string& _pointName);
    void RemoveKeyframePoint(int _pointIndex);
    void MovePointUp(int _pointIndex);
    void MovePointDown(int _pointIndex);

}; // class PostProcessPresetEditor

#endif // PostProcessPresetEditor_HPP_
