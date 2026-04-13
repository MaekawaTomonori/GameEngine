#include "DebugUI.hpp"

#include <algorithm>
#include <map>
#include <mutex>
#include <ranges>

#include "Log.hpp"
#include "Utils.hpp"
#include "ReferencePtr.hpp"

#ifdef _DEBUG
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "imnodes.h"
#endif

#undef min
#undef max

DebugUI::~DebugUI() {
#ifdef _DEBUG
    ImNodes::DestroyContext();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif
}

void DebugUI::Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter) {
    if (!_adapter) {
        Utils::Alert("DirectXAdapter is null");
        return;
    }
#ifdef _DEBUG
    adapter_ = _adapter;
    heap_ = std::make_unique<Heap>();
    if (!heap_->Create(_adapter->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)){
        Utils::Alert("Failed to create ImGui Heap");
        return;
    }
    ImGui::CreateContext();
    ImNodes::CreateContext();
    SetupStyle();

    // windowStates_ の表示状態を imgui.ini に永続化する
    {
        ImGuiSettingsHandler handler{};
        handler.TypeName = "WindowStates";
        handler.TypeHash = ImHashStr("WindowStates");
        handler.UserData = this;
        handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*, const char*) -> void* {
            return reinterpret_cast<void*>(1);
        };
        handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler* h, void*, const char* line) {
            auto* self = static_cast<DebugUI*>(h->UserData);
            const char* eq = strchr(line, '=');
            if (!eq) return;
            std::string key(line, eq - line);
            self->windowStates_[key].visible = (atoi(eq + 1) != 0);
        };
        handler.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* h, ImGuiTextBuffer* buf) {
            const auto* self = static_cast<const DebugUI*>(h->UserData);
            buf->appendf("[%s][Data]\n", h->TypeName);
            for (const auto& [key, state] : self->windowStates_) {
                buf->appendf("%s=%d\n", key.c_str(), state.visible ? 1 : 0);
            }
            buf->appendf("\n");
        };
        ImGui::AddSettingsHandler(&handler);
    }

    // ConfigFlags は Init より前に設定する必要がある（ViewportsEnable は Init 内で参照される）
    {
        ImGuiIO& preIo = ImGui::GetIO();
        preIo.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // ImGuiConfigFlags_ViewportsEnable は UNORM_SRGB フォーマットのSwapChainが作れないため無効
    }

    ImGui_ImplDX12_InitInfo dx12Info{};
    dx12Info.Device            = _adapter->GetDevice();
    dx12Info.CommandQueue      = _adapter->GetCommandQueue();
    dx12Info.NumFramesInFlight = 2;
    dx12Info.RTVFormat         = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    dx12Info.DSVFormat         = DXGI_FORMAT_UNKNOWN;
    dx12Info.SrvDescriptorHeap = heap_->Get();
    dx12Info.UserData          = this;
    dx12Info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info,
        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
        auto* ui = static_cast<DebugUI*>(info->UserData);
        const uint32_t slot = ui->nextSrvSlot_++;
        *out_cpu = ui->heap_->GetCPUHandle(slot);
        *out_gpu = ui->heap_->GetGPUHandle(slot);
    };
    dx12Info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE) {};
    ImGui_ImplDX12_Init(&dx12Info);

    ImGui_ImplWin32_Init(_adapter->GetWindowHandle());

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.f / ImGui_ImplWin32_GetDpiScaleForHwnd(_adapter->GetWindowHandle());
    io.IniFilename = "Assets\\Config\\imgui.ini";

    io.Fonts->Clear();

    ImFont* font = io.Fonts->AddFontFromFileTTF(
        //"Assets\\Fonts\\MPLUS1p-Medium.ttf",
        "Assets\\Fonts\\Satoshi-Variable.ttf",
        18.f, nullptr, io.Fonts->GetGlyphRangesJapanese()
    );

    if (font == nullptr) {
        Log::Send(Log::Level::WARNING, "Failed to load Jp font. Using Default Font.");

        io.Fonts->AddFontDefault();
        io.Fonts->GetGlyphRangesJapanese();
    }else {
        Log::Send(Log::Level::INFO, "Successfully loaded Jp Font.");
    }

    io.Fonts->Build();

    showMenuBar_ = true;
#endif
}

void DebugUI::Process() {
#ifdef _DEBUG
    std::vector<Command> commands = commands_;
    commands_.clear();

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // DockSpace - keeping original configuration
    ImGui::DockSpaceOverViewport(ImGui::GetID(""), ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    RenderMainMenuBar();

    std::ranges::sort(commands, [](const Command& _a, const Command& _b){
        return _a.id < _b.id;
    });

    for (const auto &[id, command] : commands) {
        if (windowStates_.contains(id) && !windowStates_[id].visible) {
            continue;
        }

        command();
    }

    ImGui::EndFrame();
    ImGui::Render();
#endif
}

void DebugUI::Render() {
#ifdef _DEBUG
    Process();

    ID3D12DescriptorHeap* heaps[] = {heap_->Get()};
    adapter_->GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), adapter_->GetCommandList());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
#endif
}

void DebugUI::RegisterCommand([[maybe_unused]] const std::string &_id, [[maybe_unused]] std::function<void()> _command) {
#ifdef _DEBUG
    std::lock_guard lock(mutex_);
    commands_.push_back({.id= _id, .command= std::move(_command)});
#endif
}


void DebugUI::SetToolbarContent([[maybe_unused]] std::function<void()> _content) {
#ifdef _DEBUG
    toolbarContent_ = std::move(_content);
#endif
}

void DebugUI::RegisterMenuButton([[maybe_unused]] const std::string& _key, [[maybe_unused]] const bool _flag, [[maybe_unused]]const std::string& _group) {
#ifdef _DEBUG
    std::lock_guard lock(mutex_);
    if (!windowStates_.contains(_key)) {
        windowStates_[_key].visible = _flag;
    }
    windowStates_[_key].group = _group;
#endif
}

bool& DebugUI::IsVisible(const std::string& _key) {
    return windowStates_[_key].visible;
}

uint64_t DebugUI::RegisterTexture([[maybe_unused]] ID3D12Resource* _resource, [[maybe_unused]] DXGI_FORMAT _format) const {
#ifdef _DEBUG
    if (!_resource || !adapter_) return 0;

    // 高アドレス側から1スロットずつ割り当て（ImGui の低アドレス側割り当てと衝突しない）
    const uint32_t slot = nextTexSlot_--;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = _format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    adapter_->GetDevice()->CreateShaderResourceView(_resource, &srvDesc, heap_->GetCPUHandle(slot));
    return heap_->GetGPUHandle(slot).ptr;
#else
    return 0;
#endif
}

void DebugUI::UpdateDisplaySize([[maybe_unused]]int _width, [[maybe_unused]]int _height) const {
#ifdef _DEBUG
    if (ImGui::GetCurrentContext()) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(_width), static_cast<float>(_height));
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // ViewportsEnable は UNORM_SRGB フォーマットのSwapChainと非互換のため有効化しない
        io.FontGlobalScale = 1.f / ImGui_ImplWin32_GetDpiScaleForHwnd(adapter_->GetWindowHandle());
        Log::Send(Log::Level::INFO, "ImGui display size updated: " + std::to_string(_width) + "x" + std::to_string(_height));
    }
#endif
}

bool DebugUI::IsMouseOverDebugUI() const {
#ifdef _DEBUG
    const ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx || !ctx->HoveredWindow) return false;
    // "Scene" ウィンドウ以外の ImGui ウィンドウにホバー中であれば true
    return strcmp(ctx->HoveredWindow->Name, "Scene") != 0;
#else
    return false;
#endif
}

void DebugUI::RenderMainMenuBar() {
#ifdef _DEBUG
    if (!showMenuBar_ || !ImGui::BeginMainMenuBar()) return;

    // WindowsMenuBar
    if (!windowStates_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Button,
            showWindowsPanel_ ? ImVec4(0.35f, 0.10f, 0.10f, 1.0f)
                              : ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.30f, 0.08f, 0.08f, 1.0f));
        if (ImGui::Button("  Windows  ")) {
            showWindowsPanel_ = !showWindowsPanel_;
            if (showWindowsPanel_) panelJustOpened_ = true;
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0.f, 4.f);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0.f, 8.f);
    }

    // Toolbar
    if (toolbarContent_) {
        constexpr float kToolbarW = 155.f; // Stop + sep + Play + Pause + Step の概算幅
        const float centerX = (ImGui::GetWindowWidth() - kToolbarW) * 0.5f - 20.f;
        const float cursorX = ImGui::GetCursorPosX() + 8.f;
        ImGui::SameLine(std::max(centerX, cursorX));
        toolbarContent_();
    }

    // Right: FPS + Close Button
    const float fps = ImGui::GetIO().Framerate;
    char fpsBuf[24];
    snprintf(fpsBuf, sizeof(fpsBuf), "%.1f fps", fps);

    const float fpsW   = ImGui::CalcTextSize(fpsBuf).x;
    const float closeW = ImGui::CalcTextSize(" x ").x
                         + ImGui::GetStyle().FramePadding.x * 2.f;
    const float gap    = 8.f;
    ImGui::SameLine(ImGui::GetWindowWidth() - fpsW - closeW - gap * 2.f);

    // FPS: 緑(≥55) / 黄(≥30) / 赤(<30)
    const ImVec4 fpsColor = fps >= 55.f ? ImVec4(0.25f, 0.82f, 0.38f, 1.0f)
                          : fps >= 30.f ? ImVec4(0.90f, 0.72f, 0.08f, 1.0f)
                                        : ImVec4(0.90f, 0.28f, 0.22f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, fpsColor);
    ImGui::TextUnformatted(fpsBuf);
    ImGui::PopStyleColor();

    ImGui::SameLine(0.f, gap);

    // 閉じるボタン: 透明 → ホバーで赤
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.45f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
    if (ImGui::SmallButton(" x ")) {
        showMenuBar_ = false;
    }
    ImGui::PopStyleColor(4);

    ImGui::EndMainMenuBar();

    if (showWindowsPanel_) {
        MenuBar();
    }
#endif
}

void DebugUI::MenuBar() {
#ifdef _DEBUG
    constexpr float kPanelW  = 210.f;
    constexpr float kRowH    = 22.f;   // 1行の高さ
    constexpr float kDotR    = 3.5f;   // インジケータ円の半径
    constexpr float kDotOffX = 14.f;   // ドット中心の左余白
    constexpr float kTextOffX = kDotOffX + kDotR + 8.f; // テキスト開始X

    const ImGuiViewport* vp      = ImGui::GetMainViewport();
    const float          barH    = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos({vp->Pos.x, vp->Pos.y + barH}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({kPanelW, 0.f}, ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints({kPanelW, 0.f}, {kPanelW, 500.f});
    ImGui::SetNextWindowBgAlpha(0.97f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 6.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0.f, 1.f));

    ImGui::Begin("##WindowsPanel", &showWindowsPanel_,
        ImGuiWindowFlags_NoTitleBar      |
        ImGuiWindowFlags_NoResize        |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_AlwaysAutoResize|
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar);

    // ── 行レンダラー ─────────────────────────────────────────────────
    //   ドット（可視状態）＋フル幅 Selectable でトグル
    auto renderRow = [&](const std::string& label, WindowState& state) {
        ImGui::PushID(label.c_str());

        ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.22f, 0.07f, 0.07f, 0.70f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.32f, 0.09f, 0.09f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.18f, 0.05f, 0.05f, 1.00f));

        if (ImGui::Selectable("##row", state.visible, 0, ImVec2(kPanelW, kRowH))) {
            state.visible = !state.visible;
        }

        ImGui::PopStyleColor(3);

        // Selectable の矩形に重ねてドットとラベルを描画
        const ImVec2 rMin = ImGui::GetItemRectMin();
        const ImVec2 rMax = ImGui::GetItemRectMax();
        const float  midY = (rMin.y + rMax.y) * 0.5f;

        ImDrawList* dl = ImGui::GetWindowDrawList();

        // ドット: 表示中=緑塗り / 非表示=グレー輪郭
        const ImVec2 dotPos = {rMin.x + kDotOffX, midY};
        if (state.visible) {
            dl->AddCircleFilled(dotPos, kDotR, IM_COL32(52, 188, 74, 255));
        } else {
            dl->AddCircle(dotPos, kDotR, IM_COL32(80, 80, 80, 200), 12, 1.5f);
        }

        // ラベル: 表示中は明るく、非表示は薄く
        const ImU32 textCol = state.visible
            ? IM_COL32(220, 220, 220, 255)
            : IM_COL32(95,  95,  95,  255);
        const float textY = midY - ImGui::GetFontSize() * 0.5f;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
            {rMin.x + kTextOffX, textY}, textCol, label.c_str());

        ImGui::PopID();
    };

    // ── グループヘッダー ──────────────────────────────────────────────
    auto renderGroupHeader = [&](const std::string& name) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 4.f));
        ImGui::SetCursorPosX(kDotOffX - kDotR);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.38f, 0.38f, 0.38f, 1.0f));
        ImGui::TextUnformatted(name.c_str());
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    };

    // ── グループ分け ─────────────────────────────────────────────────
    std::map<std::string, std::vector<std::pair<const std::string*, WindowState*>>> groups;
    std::vector<std::pair<const std::string*, WindowState*>> ungrouped;

    for (auto& [label, state] : windowStates_) {
        if (state.group.empty()) ungrouped.emplace_back(&label, &state);
        else                     groups[state.group].emplace_back(&label, &state);
    }

    // グループ付きアイテム
    bool firstGroup = true;
    for (auto& [groupName, items] : groups) {
        if (!firstGroup) ImGui::Spacing();
        firstGroup = false;

        renderGroupHeader(groupName);
        for (auto& [label, state] : items) renderRow(*label, *state);
    }

    // グループなしアイテム（あればセパレータで区切る）
    if (!ungrouped.empty()) {
        if (!groups.empty()) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.16f, 0.16f, 0.16f, 1.0f));
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }
        for (auto& [label, state] : ungrouped) renderRow(*label, *state);
    }

    ImGui::Spacing();

    const ImVec2 panelPos  = ImGui::GetWindowPos();
    const ImVec2 panelSize = ImGui::GetWindowSize();
    ImGui::PopStyleVar(2);
    ImGui::End();

    // パネル外クリックで閉じる（開いた直後のフレームはスキップ）
    if (panelJustOpened_) {
        panelJustOpened_ = false;
    } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        const ImVec2 m = ImGui::GetMousePos();
        const bool inPanel =
            m.x >= panelPos.x && m.x <= panelPos.x + panelSize.x &&
            m.y >= panelPos.y && m.y <= panelPos.y + panelSize.y;
        if (!inPanel) showWindowsPanel_ = false;
    }
#endif
}

void DebugUI::SetupStyle() {
#ifdef _DEBUG
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Ultra Dark Theme Colors
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.02f, 0.02f, 0.02f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.01f, 0.01f, 0.01f, 0.50f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.70f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.60f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.80f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.50f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.40f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.08f, 0.08f, 0.08f, 0.55f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.12f, 0.12f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.60f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.50f, 0.15f, 0.15f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.50f, 0.15f, 0.15f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.60f, 0.15f, 0.15f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.70f, 0.20f, 0.20f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.45f, 0.12f, 0.12f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.15f, 0.05f, 0.05f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.90f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.90f, 0.35f);

    // Spacing (spacing.small=4 / spacing.medium=8 / spacing.large=16)
    style.WindowPadding     = ImVec2(8.00f, 8.00f);
    style.FramePadding      = ImVec2(6.00f, 3.00f);
    style.CellPadding       = ImVec2(6.00f, 6.00f);
    style.ItemSpacing       = ImVec2(6.00f, 5.00f);
    style.ItemInnerSpacing  = ImVec2(5.00f, 4.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing     = 18;
    style.ScrollbarSize     = 12;
    style.GrabMinSize       = 8;

    // Borders
    style.WindowBorderSize  = 1;
    style.ChildBorderSize   = 1;
    style.PopupBorderSize   = 1;
    style.FrameBorderSize   = 0;  // フレーム枠を外してフラットに
    style.TabBorderSize     = 0;

    // Rounding
    style.WindowRounding    = 6;
    style.ChildRounding     = 4;
    style.FrameRounding     = 3;
    style.PopupRounding     = 4;
    style.ScrollbarRounding = 6;
    style.GrabRounding      = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding       = 3;
#endif
}
