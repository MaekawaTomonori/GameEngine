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
    std::vector<Command> commands = commands_;
    commands_.clear(); 

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // DockSpace
    ImGui::DockSpaceOverViewport(ImGui::GetID(""), ImGui::GetMainViewport());

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


