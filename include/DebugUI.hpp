#ifndef DebugUI_HPP_
#define DebugUI_HPP_
#include <memory>

#include "src/Renderer/DirectX/DirectXAdapter.hpp"
#include "src/Renderer/DirectX/Heap/Heap.hpp"

//ImGui Adapter
class DebugUI {
    std::unique_ptr<Heap> heap_;
    ID3D12GraphicsCommandList* cList_ = nullptr;

    //std::vector<PROCESS> processes_;
public:
    ~DebugUI();
    void Initialize(const DirectXAdapter* dx);
    void Render();

public:
    //void AddProcess(PROCESS);
private:
    void Process();
}; // class DebugUI

#endif // DebugUI_HPP_
