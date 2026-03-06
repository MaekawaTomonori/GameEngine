#ifndef WatchDebugger_HPP_
#define WatchDebugger_HPP_

#ifdef _DEBUG

#include <format>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "Pattern/Singleton.hpp"

class DebugUI;
class JsonParams;

/** @brief 変数ウォッチデバッガー
 ** ENGINE_WATCH / ENGINE_WATCH_EDIT でリアルタイム表示・編集、
 ** ENGINE_WATCH_GROUP_BEGIN / END でグループ化する
 **
 **   ENGINE_WATCH_GROUP_BEGIN("Player");
 **   ENGINE_WATCH     ("HP",    &hp_);
 **   ENGINE_WATCH_EDIT("Speed", &speed_);                   // 編集のみ
 **   ENGINE_WATCH_EDIT("Speed", &speed_, "enemy");           // 終了時に Assets/Data/enemy.json へ保存
 **   ENGINE_WATCH_EDIT("Speed", &speed_, "enemy", "spd");    // キー名を明示
 **   ENGINE_WATCH_GROUP_END();
 **/
class WatchDebugger {
public:
    enum class EditType { Float, Int, Uint16, Uint32, Bool, Vec2, Vec3, Vec4 };
    using WatchValue = std::variant<float, int32_t, uint16_t, uint32_t, bool, Vector2, Vector3, Vector4>;

private:
    enum class EntryKind { GroupBegin, GroupEnd, ReadOnly, Editable };

    struct JsonContext {
        JsonParams* json;
        std::string name;
        std::string group;
    };

    struct Entry {
        EntryKind kind;
        std::string groupLabel;
        std::string label;

        std::function<std::string()> getter;        // ReadOnly

        void* ptr = nullptr;  // Editable
        EditType editType = EditType::Float;
        std::optional<JsonContext> jsonCtx;             // GROUP_BEGIN 経由 JSON

        std::string jsonFile;        // 独立 JSON 保存先（空なら保存しない）
        std::string jsonKey;         // JSON キー（空なら label を検索に使用）
        WatchValue cached{ 0.f };   // 終了時保存用キャッシュ
    };

    struct GroupState {
        bool active = false;
        std::string label;
        JsonParams* json = nullptr;
        std::string name;
        std::string group;
    };

    std::vector<Entry> entries_;
    GroupState currentGroup_;
    DebugUI* debugUI_ = nullptr;

    void AddReadOnly(const std::string& _label, std::function<std::string()> _getter);
    void AddEditable(const std::string& _label, void* _ptr, EditType _type, const std::string& _jsonFile, const std::string& _jsonKey);
    void AddGroupBegin(const std::string& _label);
    void AddGroupEnd();

    void RenderReadOnly(const Entry& _e);
    void RenderEditable(Entry& _e);
    void WriteToJson(const Entry& _e);

    void SaveJsonFiles();
    
    static WatchValue ReadFromPtr(void* _ptr, EditType _type);

    template<typename T>
    static EditType DeduceType() {
        if constexpr (std::is_same_v<T, float>) return EditType::Float;
        else if constexpr (std::is_same_v<T, int32_t>) return EditType::Int;
        else if constexpr (std::is_same_v<T, uint16_t>) return EditType::Uint16;
        else if constexpr (std::is_same_v<T, uint32_t>) return EditType::Uint32;
        else if constexpr (std::is_same_v<T, bool>) return EditType::Bool;
        else if constexpr (std::is_same_v<T, Vector2>) return EditType::Vec2;
        else if constexpr (std::is_same_v<T, Vector3>) return EditType::Vec3;
        else return EditType::Vec4;
    }

public:
    /** @brief 初期化
     ** @param _debug DebugUI
     **/
    void Initialize(DebugUI* _debug);

    /** @brief jsonFile 付きエントリを JSON に書き出す
     **/
    void Finalize();

    /** @brief ImGui ウィンドウを登録する
     **/
    void Debug();

    /** @brief グループ開始
     ** @param _label グループラベル
     **/
    static void BeginGroup(const std::string& _label) {
        auto* inst = Singleton<WatchDebugger>::GetInstance();
        inst->currentGroup_ = { true, _label, nullptr, "", "" };
        inst->AddGroupBegin(_label);
    }

    /** @brief グループ開始（JSON 書き込みあり）
     ** @param _label グループラベル
     ** @param _json   JsonParams インスタンス
     ** @param _name   JSON ファイル名
     ** @param _group  JSON グループキー
     **/
    static void BeginGroup(const std::string& _label, JsonParams* _json, const std::string& _name, const std::string& _group) {
        auto* inst = Singleton<WatchDebugger>::GetInstance();
        inst->currentGroup_ = { true, _label, _json, _name, _group };
        inst->AddGroupBegin(_label);
    }

    /** @brief グループ終了 
     **/
    static void EndGroup() {
        auto* inst = Singleton<WatchDebugger>::GetInstance();
        inst->currentGroup_.active = false;
        inst->AddGroupEnd();
    }

    /** @brief 読み取り専用ウォッチ
     ** @param _label ラベル
     ** @param _ptr   ポインタ
     **/
    template<typename T>
    static void Watch(const std::string& _label, const T* _ptr) {
        Singleton<WatchDebugger>::GetInstance()->AddReadOnly(_label, [_ptr]() -> std::string {
            if constexpr (std::is_same_v<T, bool>) {
                return *_ptr ? "true" : "false";
            } else if constexpr (std::is_same_v<T, Vector2>) {
                return std::format("{{{:.3f}, {:.3f}}}", _ptr->x, _ptr->y);
            } else if constexpr (std::is_same_v<T, Vector3>) {
                return std::format("{{{:.3f}, {:.3f}, {:.3f}}}", _ptr->x, _ptr->y, _ptr->z);
            } else if constexpr (std::is_same_v<T, Vector4>) {
                return std::format("{{{:.3f}, {:.3f}, {:.3f}, {:.3f}}}", _ptr->x, _ptr->y, _ptr->z, _ptr->w);
            } else if constexpr (std::is_floating_point_v<T>) {
                return std::format("{:.3f}", *_ptr);
            } else {
                return std::to_string(*_ptr);
            }
        });
    }

    /** @brief 編集可能ウォッチ
     ** @param _label ラベル
     ** @tparam T 対応型（float, int32_t, uint16_t, uint32_t, bool, Vector2, Vector3, Vector4 のいずれか）
     ** @param _ptr 
     ** @param _jsonFile 保存先ファイル
     ** @param _jsonKey JSON キー
     **/
    template<typename T>
    static void Watch(const std::string& _label, T* _ptr, const std::string& _jsonFile = "", const std::string& _jsonKey  = "") {
        WatchEdit(_label, _ptr, _jsonFile, _jsonKey);
    }

    /** @brief 編集可能ウォッチ
     ** @param _label ラベル
     ** @tparam T 対応型（float, int32_t, uint16_t, uint32_t, bool, Vector2, Vector3, Vector4 のいずれか）
     ** @param _ptr 
     ** @param _jsonFile 保存先ファイル
     ** @param _jsonKey JSON キー
     **/
    template<typename T>
    static void WatchEdit(const std::string& _label, T* _ptr, const std::string& _jsonFile = "", const std::string& _jsonKey  = "") {
        static_assert(
            std::is_same_v<T, float>    || std::is_same_v<T, int32_t>  ||
            std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> ||
            std::is_same_v<T, bool>     || std::is_same_v<T, Vector2>  ||
            std::is_same_v<T, Vector3>  || std::is_same_v<T, Vector4>,
            "WatchEdit: unsupported type."
        );
        Singleton<WatchDebugger>::GetInstance()->AddEditable(_label, _ptr, DeduceType<T>(), _jsonFile, _jsonKey);
    }
};

#endif // _DEBUG
#endif // WatchDebugger_HPP_
