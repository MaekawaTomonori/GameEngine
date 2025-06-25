#include "DebugUI.hpp"

#include <algorithm>
#include <mutex>

#include "imgui_internal.h"
#include "include/Utils.hpp"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_dx12.h"
#include "vendor/imgui/imgui_impl_win32.h"

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
    ImGui::StyleColorsDark();

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

    cList_ = dx->GetCommandList();
}

void DebugUI::Process() {
    std::lock_guard<std::mutex> lock(mutex_);
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // DockSpace
    DockSpace();

    ImGui::ShowDemoWindow();

    std::ranges::sort(commands_, [](const Command& a, const Command& b){
        return a.id < b.id;
    });
    
    for (const auto &[id, command] : commands_) {
        command();
    }

    ImGui::Render();
    ImGui::UpdatePlatformWindows();

    cache_ = ImGui::GetDrawData();
}

void DebugUI::Render() const {
    if (!cache_)return;

    ID3D12DescriptorHeap* heaps[] = {heap_->Get()};
    cList_->SetDescriptorHeaps(_countof(heaps), heaps);
    ImGui_ImplDX12_RenderDrawData(cache_, cList_);
}

void DebugUI::RegisterCommand(const std::string &id, std::function<void()> _command) {
    commands_.push_back({id, std::move(_command)});
}

void DebugUI::DockSpace() {
    ImGui::DockSpaceOverViewport(ImGui::GetID(""), ImGui::GetMainViewport());
    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoResize);
    ImGuiID dockSpaceId = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockSpaceId, ImGui::GetContentRegionAvail(), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode);

    if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockSpaceId))node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar; // Disable tab bar in the central node
    ImGui::End();
}

