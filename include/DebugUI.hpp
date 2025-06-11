#ifndef DebugUI_HPP_
#define DebugUI_HPP_
#include <memory>
#include <mutex>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"

struct ImDrawData;

//ImGui Adapter
class DebugUI {
    std::mutex mutex_;

    std::unique_ptr<Heap> heap_;
    ID3D12GraphicsCommandList* cList_ = nullptr;

    //std::vector<PROCESS> processes_;

    ImDrawData* cache_ = nullptr;
public:
    ~DebugUI();
    void Initialize(const DirectXAdapter* dx);
    void Process();
    void Render();

public:
    //void AddProcess(PROCESS);
}; // class DebugUI

#endif // DebugUI_HPP_
