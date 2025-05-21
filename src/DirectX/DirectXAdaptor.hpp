#ifndef DirectXAdaptor_HPP_
#define DirectXAdaptor_HPP_

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

class DirectXAdaptor {
	Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

public:
    DirectXAdaptor();

private:
	bool CreateDXGI();

}; // class DirectXAdaptor

#endif // DirectXAdaptor_HPP_
