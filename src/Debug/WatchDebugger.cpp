#ifdef _DEBUG

#include "WatchDebugger.hpp"

#include <filesystem>
#include <fstream>
#include <map>

#include "DebugUI.hpp"
#include "imgui.h"
#include "json.hpp"
#include "src/Json/JsonParams.hpp"

/** JSON ノードを再帰的に検索し、最初に見つかった _key の値を _val で上書きする **/
static bool UpdateJsonNode(nlohmann::json& _node,
                           const std::string& _key,
                           const WatchDebugger::WatchValue& _val) {
    if (!_node.is_object()) return false;

    if (_node.contains(_key)) {
        std::visit([&](auto&& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, Vector2>) {
                _node[_key] = { v.x, v.y };
            } else if constexpr (std::is_same_v<T, Vector3>) {
                _node[_key] = { v.x, v.y, v.z };
            } else if constexpr (std::is_same_v<T, Vector4>) {
                _node[_key] = { v.x, v.y, v.z, v.w };
            } else if constexpr (std::is_same_v<T, bool>) {
                _node[_key] = static_cast<int32_t>(v);
            } else {
                _node[_key] = v;
            }
        }, _val);
        return true;
    }

    for (auto& [k, child] : _node.items()) {
        if (UpdateJsonNode(child, _key, _val)) return true;
    }
    return false;
}


void WatchDebugger::AddReadOnly(const std::string& _label, std::function<std::string()> _getter) {
    const std::string key = currentGroup_.active ? currentGroup_.label + "|" + _label : _label;
    for (auto& e : entries_) {
        if (e.kind == EntryKind::ReadOnly && e.groupLabel + "|" + e.label == key) {
            e.getter = std::move(_getter);
            return;
        }
    }
    Entry e;
    e.kind       = EntryKind::ReadOnly;
    e.groupLabel = currentGroup_.active ? currentGroup_.label : "";
    e.label      = _label;
    e.getter     = std::move(_getter);
    entries_.push_back(std::move(e));
}

void WatchDebugger::AddEditable(const std::string& _label, void* _ptr, EditType _type, const std::string& _jsonFile, const std::string& _jsonKey) {
    const std::string key = currentGroup_.active ? currentGroup_.label + "|" + _label : _label;

    std::optional<JsonContext> ctx;
    if (currentGroup_.active && currentGroup_.json) {
        ctx = JsonContext{ currentGroup_.json, currentGroup_.name, currentGroup_.group };
    }

    for (auto& e : entries_) {
        if (e.kind == EntryKind::Editable && e.groupLabel + "|" + e.label == key) {
            e.ptr      = _ptr;
            e.editType = _type;
            e.jsonCtx  = ctx;
            e.jsonFile = _jsonFile;
            e.jsonKey  = _jsonKey;
            e.cached   = ReadFromPtr(_ptr, _type);
            return;
        }
    }
    Entry e;
    e.kind       = EntryKind::Editable;
    e.groupLabel = currentGroup_.active ? currentGroup_.label : "";
    e.label      = _label;
    e.ptr        = _ptr;
    e.editType   = _type;
    e.jsonCtx    = ctx;
    e.jsonFile   = _jsonFile;
    e.jsonKey    = _jsonKey;
    e.cached     = ReadFromPtr(_ptr, _type);
    entries_.push_back(std::move(e));
}

void WatchDebugger::AddGroupBegin(const std::string& _label) {
    for (const auto& e : entries_) {
        if (e.kind == EntryKind::GroupBegin && e.label == _label) return;
    }
    Entry e;
    e.kind  = EntryKind::GroupBegin;
    e.label = _label;
    entries_.push_back(std::move(e));
}

void WatchDebugger::AddGroupEnd() {
    Entry e;
    e.kind = EntryKind::GroupEnd;
    entries_.push_back(std::move(e));
}

void WatchDebugger::Initialize(const GESTD::ReferencePtr<DebugUI> _debug) {
    debugUI_ = _debug;
    if (debugUI_) {
        debugUI_->RegisterMenuButton("Watcher", false, "Debug");
    }
}

void WatchDebugger::Finalize() {
    SaveJsonFiles();
}

// ─── Static helper ───────────────────────────────────────

WatchDebugger::WatchValue WatchDebugger::ReadFromPtr(void* _ptr, EditType _type) {
    switch (_type) {
    case EditType::Float:  return *static_cast<float*>   (_ptr);
    case EditType::Int:    return *static_cast<int32_t*> (_ptr);
    case EditType::Uint16: return *static_cast<uint16_t*>(_ptr);
    case EditType::Uint32: return *static_cast<uint32_t*>(_ptr);
    case EditType::Bool:   return *static_cast<bool*>    (_ptr);
    case EditType::Vec2:   return *static_cast<Vector2*> (_ptr);
    case EditType::Vec3:   return *static_cast<Vector3*> (_ptr);
    case EditType::Vec4:   return *static_cast<Vector4*> (_ptr);
    }
    return 0.f;
}

// ─── Render helpers ──────────────────────────────────────

void WatchDebugger::RenderReadOnly(const Entry& _e) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(_e.label.c_str());
    ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(_e.getter().c_str());
}

void WatchDebugger::RenderEditable(Entry& _e) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(_e.label.c_str());
    ImGui::TableSetColumnIndex(1);

    const std::string id = "##" + _e.groupLabel + _e.label;
    bool changed = false;

    switch (_e.editType) {
    case EditType::Float:  changed = ImGui::DragFloat (id.c_str(), static_cast<float*>   (_e.ptr), 0.01f); break;
    case EditType::Int:    changed = ImGui::DragInt   (id.c_str(), static_cast<int32_t*> (_e.ptr));        break;
    case EditType::Uint16: changed = ImGui::DragScalar(id.c_str(), ImGuiDataType_U16, _e.ptr);             break;
    case EditType::Uint32: changed = ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, _e.ptr);             break;
    case EditType::Bool:   changed = ImGui::Checkbox  (id.c_str(), static_cast<bool*>    (_e.ptr));        break;
    case EditType::Vec2:   changed = ImGui::DragFloat2(id.c_str(), &static_cast<Vector2*>(_e.ptr)->x, 0.01f); break;
    case EditType::Vec3:   changed = ImGui::DragFloat3(id.c_str(), &static_cast<Vector3*>(_e.ptr)->x, 0.01f); break;
    case EditType::Vec4:   changed = ImGui::DragFloat4(id.c_str(), &static_cast<Vector4*>(_e.ptr)->x, 0.01f); break;
    }

    // キャッシュを常に最新値に更新（デストラクタ時の JSON 保存に使用）
    _e.cached = ReadFromPtr(_e.ptr, _e.editType);

    if (changed && _e.jsonCtx) {
        WriteToJson(_e);
    }
}

void WatchDebugger::WriteToJson(const Entry& _e) {
    auto& ctx = *_e.jsonCtx;
    std::visit([&](auto&& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, bool>     ||
                      std::is_same_v<T, uint16_t> ||
                      std::is_same_v<T, uint32_t>) {
            ctx.json->SetValue(ctx.name, ctx.group, _e.label, static_cast<int32_t>(v));
        } else {
            ctx.json->SetValue(ctx.name, ctx.group, _e.label, v);
        }
    }, ReadFromPtr(_e.ptr, _e.editType));
}

// ─── JSON 保存（デストラクタ） ────────────────────────────

void WatchDebugger::SaveJsonFiles() {
    // jsonFile を持つエントリをファイルごとにまとめる
    std::map<std::string, std::vector<const Entry*>> byFile;
    for (const auto& e : entries_) {
        if (e.kind == EntryKind::Editable && !e.jsonFile.empty()) {
            byFile[e.jsonFile].push_back(&e);
        }
    }

    for (const auto& [file, list] : byFile) {
        const std::string path = "Assets/Data/" + file + ".json";

        nlohmann::json root;
        {
            std::ifstream in(path);
            if (!in.is_open()) continue; // ファイルが存在しない場合はスキップ
            in >> root;
        }

        for (const auto* e : list) {
            const std::string key = e->jsonKey.empty() ? e->label : e->jsonKey;
            UpdateJsonNode(root, key, e->cached);
        }

        std::ofstream out(path);
        if (out.is_open()) {
            out << root.dump(4) << '\n';
        }
    }
}

// ─── Debug ───────────────────────────────────────────────

void WatchDebugger::Debug() {
    if (!debugUI_) return;

    debugUI_->RegisterCommand("Watcher", [this]() {
        ImGui::Begin("Watcher", &debugUI_->IsVisible("Watcher"));

        if (entries_.empty()) {
            ImGui::TextDisabled("No watches registered.");
            ImGui::End();
            return;
        }

        bool inTable   = false;
        bool groupOpen = true;
        int  tableIdx  = 0;

        auto beginTable = [&]() {
            inTable = ImGui::BeginTable(("##WT" + std::to_string(tableIdx++)).c_str(), 2,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame);
            if (inTable) {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
            }
        };
        auto endTable = [&]() {
            if (inTable) { ImGui::EndTable(); inTable = false; }
        };

        // GroupBegin の後に実エントリが存在するか先読みする
        auto groupHasEntries = [&](size_t _from) {
            for (size_t j = _from + 1; j < entries_.size(); ++j) {
                if (entries_[j].kind == EntryKind::GroupEnd)   return false;
                if (entries_[j].kind == EntryKind::ReadOnly ||
                    entries_[j].kind == EntryKind::Editable)   return true;
            }
            return false;
        };

        for (size_t i = 0; i < entries_.size(); ++i) {
            auto& e = entries_[i];
            switch (e.kind) {
            case EntryKind::GroupBegin:
                endTable();
                if (groupHasEntries(i)) {
                    groupOpen = ImGui::CollapsingHeader(e.label.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
                } else {
                    groupOpen = false; // エントリなし → ヘッダーを表示しない
                }
                break;

            case EntryKind::GroupEnd:
                endTable();
                groupOpen = true;
                break;

            case EntryKind::ReadOnly:
            case EntryKind::Editable:
                if (!groupOpen) break;
                if (!inTable) beginTable();
                if (!inTable) break;
                if (e.kind == EntryKind::ReadOnly) RenderReadOnly(e);
                else                               RenderEditable(e);
                break;
            }
        }
        endTable();

        ImGui::End();
    });
}

#endif // _DEBUG
