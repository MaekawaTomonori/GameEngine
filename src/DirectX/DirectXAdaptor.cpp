#include "DirectXAdaptor.hpp"

#include "include/Log.hpp"
#include "include/Utils.hpp"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

DirectXAdaptor::DirectXAdaptor() {
	if (CreateDXGI()) {
		Log::Send(Log::Level::INFO, "DXGI Created");
	} else{
		Utils::Alert("Failed to create DXGI");
	}
}

bool DirectXAdaptor::CreateDXGI() {
	Log::Send(Log::Level::INFO, "Create DXGI");

	//Factory
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory_)))){
		return false;
	}

	Log::Send(Log::Level::INFO, "Factory Created");

	//Adaptor
	for (UINT i = 0; factory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter_)); ++i){
		// Check if the adapter supports Direct3D 12
		DXGI_ADAPTER_DESC3 desc;
		if (FAILED(adapter_->GetDesc3(&desc))){
			adapter_ = nullptr;
		}
		if (!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)){
			break;
		}
		adapter_ = nullptr;
	}

	if (adapter_ == nullptr)return false;

	Log::Send(Log::Level::INFO, "Adapter Created");

	//Device
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelNames[] = {
		"12.2",
		"12.1",
		"12.0"
	};

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		if (SUCCEEDED(D3D12CreateDevice(adapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_)))){
			Log::Send(Log::Level::INFO, "Device Created");
			Log::Send(Log::Level::INFO, "Feature Level: " + std::string(featureLevelNames[i]));
			break;
		}

		Log::Send(Log::Level::ERR, "Failed to create device with feature level: " + std::string(featureLevelNames[i]));
	}

	if (device_ == nullptr){
		Log::Send(Log::Level::ERR, "Failed to create device");
		return false;
	}

	Log::Send(Log::Level::INFO, "Complete create DXGI");
	return true;
}
