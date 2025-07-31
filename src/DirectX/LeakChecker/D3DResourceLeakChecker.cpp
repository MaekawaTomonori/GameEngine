#include "D3DResourceLeakChecker.hpp"

#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl/client.h>

#pragma comment(lib, "dxguid.lib")

D3DResourceLeakChecker::~D3DResourceLeakChecker() {
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))){
		OutputDebugStringA("=== D3D12 Resource Leak Report ===\n");
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		OutputDebugStringA("=== DXGI Resource Leak Report ===\n");
		debug->ReportLiveObjects(DXGI_DEBUG_DXGI, static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		OutputDebugStringA("=== App Resource Leak Report ===\n");
		debug->ReportLiveObjects(DXGI_DEBUG_APP, static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}
}
