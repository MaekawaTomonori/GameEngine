#pragma once

#include <dxcapi.h>
#include <string>
#include <wrl/client.h>

class Shader{
	//DXC
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	//Shader
	std::wstring name_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShader_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShader_;
	IDxcBlob* Compile(const std::wstring& directoryPath_, const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

	void CreateDxc();
	void CompileShaders();

public:
	bool Create(const std::wstring& name);

    Shader* PSLoad(const std::wstring& name);

    IDxcBlob* GetVertexShader() const {
        return vertexShader_.Get();
    }
    IDxcBlob* GetPixelShader() const {
        return pixelShader_.Get();
    }
};


