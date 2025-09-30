#include "DebugUI.hpp"

#include <algorithm>
#include <mutex>

#include "include/Utils.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

DebugUI::~DebugUI() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void DebugUI::Initialize(const DirectXAdapter *dx) {
    if (!dx) {
        Utils::Alert("DirectXAdapter is null");
        return;
    }
    heap_ = std::make_unique<Heap>();
    if (!heap_->Create(dx->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)){
        Utils::Alert("Failed to create ImGui Heap");
        return;
    }
    ImGui::CreateContext();
    SetupModernStyle();

    ImGui_ImplDX12_Init(
        dx->GetDevice(), 
        2,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 
        heap_->Get(), 
        heap_->Get()->GetCPUDescriptorHandleForHeapStart(), 
        heap_->Get()->GetGPUDescriptorHandleForHeapStart()
    );

    ImGui_ImplWin32_Init(dx->GetWindowHandle());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    io.FontGlobalScale = 1.f / ImGui_ImplWin32_GetDpiScaleForHwnd(dx->GetWindowHandle());
    io.IniFilename = "Assets\\Config\\imgui.ini"; 

    cList_ = dx->GetCommandList();
}

void DebugUI::Process() {
    std::vector<Command> commands = commands_;
    commands_.clear(); 

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // DockSpace - keeping original configuration
    ImGui::DockSpaceOverViewport(ImGui::GetID(""), ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    // Custom menu bar with FPS display
    if (ImGui::BeginMainMenuBar()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
        ImGui::Text("Game Engine Debug Interface");
        ImGui::PopStyleColor();
        
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 0.4f, 1.0f));
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::PopStyleColor();
        
        ImGui::EndMainMenuBar();
    }

    ImGui::ShowDemoWindow();

    std::ranges::sort(commands, [](const Command& a, const Command& b){
        return a.id < b.id;
    });
    
    for (const auto &[id, command] : commands) {
        command();
    }

    ImGui::EndFrame();
    ImGui::Render();
}

void DebugUI::Render() {
    Process();

    ID3D12DescriptorHeap* heaps[] = {heap_->Get()};
    cList_->SetDescriptorHeaps(_countof(heaps), heaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cList_);
}

void DebugUI::RegisterCommand(const std::string &_id, std::function<void()> _command) {
    std::lock_guard<std::mutex> lock(mutex_);
    commands_.push_back({.id= _id, .command= std::move(_command)});
}

void DebugUI::SetupModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Ultra Dark Theme Colors
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.02f, 0.02f, 0.02f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
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
    colors[ImGuiCol_Separator]              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
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

    // Modern styling
    style.WindowPadding     = ImVec2(8.00f, 8.00f);
    style.FramePadding      = ImVec2(5.00f, 2.00f);
    style.CellPadding       = ImVec2(6.00f, 6.00f);
    style.ItemSpacing       = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing  = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing     = 25;
    style.ScrollbarSize     = 15;
    style.GrabMinSize       = 10;
    style.WindowBorderSize  = 1;
    style.ChildBorderSize   = 1;
    style.PopupBorderSize   = 1;
    style.FrameBorderSize   = 1;
    style.TabBorderSize     = 1;
    style.WindowRounding    = 7;
    style.ChildRounding     = 4;
    style.FrameRounding     = 3;
    style.PopupRounding     = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding      = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding       = 4;
}


