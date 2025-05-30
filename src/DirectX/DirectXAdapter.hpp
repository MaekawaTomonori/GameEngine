#ifndef DirectXAdapter_HPP_
#define DirectXAdapter_HPP_

#include <d3d12.h>
#include <dxgi1_6.h>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <wrl/client.h>

#include "Heap/Heap.hpp"
#include "include/Math/Vector4.hpp"
#include "src/Timer/Timer.hpp"

class FrameRateLimiter{
	uint16_t maxFps_;
	std::chrono::steady_clock::time_point reference_;
	bool vsyncEnabled_;

public:
	explicit FrameRateLimiter(uint16_t maxFps, bool useVsync = true);

	void WaitForNextFrame();
};

class DirectXAdapter {


	using WindowSize = std::pair<size_t, size_t>;
	WindowSize windowSize_ = {800, 600};
	HWND hWnd_ = nullptr;

	//DegubLayer
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugLayer_;

	//DXGIs
	Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	//Command
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> cQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cList_;

	//SwapChain
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	//RTV
	std::unique_ptr<Heap> rtvHeap_;

	//Resource
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainResources_;

	//RtvHandle
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

	//Background color
	Vector4	back = {0.2f, 0.2f, 0.2f, 1.0f}; // Black

	//Fence
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = nullptr;

	//RenderingCommand
	std::vector<std::function<void()>> renderingCommands_;

	std::unique_ptr<FrameRateLimiter> fpsLimiter_ = nullptr;

public:
    DirectXAdapter(HWND _hWnd, size_t _width, size_t _height);

	void Register(std::function<void()> _task);
	void Render();
private:
	void EnableDebugLayer();
	bool CreateDXGI();
	bool InfoQueue() const;
	bool CreateCommand();
	bool CreateSwapChain();
	bool CreateRTV();
	bool CreateFence();
	void Wait();
public: //Accessor
	HWND GetWindowHandle() const;
	[[nodiscard]] ID3D12Device *GetDevice() const;
	[[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const;
}; // class DirectXAdapter

#endif // DirectXAdaptor_HPP_
