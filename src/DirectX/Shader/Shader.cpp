#include "Shader.h"

#include <cassert>
#include <format>

#include "Log.hpp"
#include "Utils.hpp"

void Shader::CreateDxc() {
    HRESULT hr = S_OK;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    assert(SUCCEEDED(hr));

    hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    assert(SUCCEEDED(hr));
}

void Shader::CompileShaders() {
    vertexShader_.Attach(Compile(L"Assets/Shaders/", name_ + L".VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    pixelShader_.Attach(Compile(L"Assets/Shaders/", name_ + L".PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));

    /*if (name_ == L"Particle"){
        geometryShader_.Attach(Compile(L"Assets/Shaders/", name_ + L".GS.hlsl", L"gs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    }*/
}

bool Shader::Create(const std::wstring& name) {
    name_ = name;

    CreateDxc();
    CompileShaders();

    return true;
}

Shader* Shader::PSLoad(const std::wstring& name) {
    pixelShader_.Attach(Compile(L"Assets/Shaders/", name + L".hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    return this;
}

IDxcBlob* Shader::Compile(const std::wstring& directoryPath, const std::wstring& filePath, const wchar_t* profile,
                          IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
    Log::Send(Log::Level::INFO, Utils::Convert(std::format(L"Begin CompileShader, Path : {}, Profile : {}", filePath, profile)));
    HRESULT hResult = S_FALSE;
    IDxcBlobEncoding* shaderSource = nullptr;
    std::wstring fullPath = directoryPath + filePath;
    hResult = dxcUtils->LoadFile(fullPath.c_str(), nullptr, &shaderSource);
    if (FAILED(hResult)) {
        Log::Send(Log::Level::ERR, Utils::Convert(std::format(L"Failed to load shader file: {}", fullPath)));
        Utils::Alert("Failed to load shader file: " + Utils::Convert(fullPath));
    }
   
    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;

    ///Compiling
    LPCWSTR arguments[] = {
        filePath.c_str(), 
        L"-E", L"main", //EntryPoint
        L"-T", profile, //ShaderProfile
        L"-I", directoryPath.c_str(), //IncludePath
        L"-Zi", L"-Qembed_debug", //DebugInfo
        L"-Od", 
        L"-Zpr", 
    };

    IDxcResult* shaderResult = nullptr;
    hResult = dxcCompiler->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        includeHandler,
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hResult));

    IDxcBlobUtf8* shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0){
        Log::Send(Log::Level::ERR, shaderError->GetStringPointer());
        Utils::Alert("Shader Compilation Error");
        assert(false);
    }

    IDxcBlob* shaderBlob = nullptr;
    hResult = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hResult));

    Log::Send(Log::Level::INFO, Utils::Convert(std::format(L"Compile Succeed, Path : {}, Profile : {}", filePath, profile)));
    shaderSource->Release();
    shaderResult->Release();

    return shaderBlob;
}

