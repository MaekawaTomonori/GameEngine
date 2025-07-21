#pragma once
#include <wrl/client.h>
#include <dxcapi.h>
#include <string>

class Shader{
    //DXC
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

    //Shader
    std::wstring name_;
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShader_;
    Microsoft::WRL::ComPtr<IDxcBlob> pixelShader_;

public:
    Shader() = default;
    Shader(const std::wstring& _name);
    bool Create(const std::wstring& _name);

    Shader* PSLoad(const std::wstring& _name);

    IDxcBlob* GetVertexShader() const {
        return vertexShader_.Get();
    }
    IDxcBlob* GetPixelShader() const {
        return pixelShader_.Get();
    }

private:
    void CreateDxc();
    void CompileShaders();
    IDxcBlob* Compile(const std::wstring& directoryPath_, const std::wstring& _filePath, const wchar_t* _profile, IDxcUtils* _dxcUtils, IDxcCompiler3* _dxcCompiler, IDxcIncludeHandler* _includeHandler);
};


