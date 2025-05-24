#ifndef DirectXAdaptor_HPP_
#define DirectXAdaptor_HPP_

#include <d3d12.h>
#include <dxgi1_6.h>
#include <utility>
#include <wrl/client.h>

class DirectXAdaptor {
	using WindowSize = std::pair<size_t, size_t>;
	WindowSize windowSize_ = {800, 600};
	HWND hWnd_ = nullptr;

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


public:
    DirectXAdaptor(HWND _hWnd, size_t _width, size_t _height);

private:
	bool CreateDXGI();
	bool CreateCommand();
	bool CreateSwapChain() const;



}; // class DirectXAdaptor

#endif // DirectXAdaptor_HPP_
