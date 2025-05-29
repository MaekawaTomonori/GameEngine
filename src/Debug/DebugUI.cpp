#include "DebugUI.hpp"

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
	if (!heap_ || !heap_->Create(dx->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)){
		Utils::Alert("Failed to create ImGui Heap");
		return;
	}
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplDX12_Init(dx->GetDevice(), 2, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, heap_->Get(), heap_->Get()->GetCPUDescriptorHandleForHeapStart(), heap_->Get()->GetGPUDescriptorHandleForHeapStart());
	ImGui_ImplWin32_Init(dx->GetWindowHandle());

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	cList_ = dx->GetCommandList();
}

void DebugUI::Process() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Do your ImGui rendering here

	// DockingSpace


	ImGui::Render();
}

void DebugUI::Render() const {
	ID3D12DescriptorHeap* heaps[] = {heap_->Get()};
	cList_->SetDescriptorHeaps(_countof(heaps), heaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cList_);
	ImGui::EndFrame();
}

