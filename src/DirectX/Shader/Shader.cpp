#include "Shader.h"

#include <cassert>
#include <format>

#include "Log.hpp"
#include "Utils.hpp"

Shader::Shader(const std::wstring& _name) {
    if (!Create(_name)) {
        Log::Send(Log::Level::ERR, Utils::Convert(std::format(L"Failed to create shader: {}", _name)));
        Utils::Alert("Failed to create shader: " + Utils::Convert(_name));
        return;
    }
    Log::Send(Log::Level::INFO, Utils::Convert(std::format(L"Shader created successfully: {}", _name)));
}

Shader::Shader(const std::wstring& _vs, const std::wstring& _ps) {
    if (!Create(_vs, _ps)){
        Log::Send(Log::Level::ERR, Utils::Convert(std::format(L"Failed to create shader: {}, {}", _vs, _ps)));
        Utils::Alert("Failed to create shader: " + Utils::Convert(_vs) + ", " + Utils::Convert(_ps));
        return;
    }
    Log::Send(Log::Level::INFO, Utils::Convert(std::format(L"Shader created successfully: {}, {}", _vs, _ps)));
}

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

void Shader::CompileShaders(const std::wstring& _vs, const std::wstring& _ps) {
    vertexShader_.Attach(Compile(L"Assets/Shaders/", _vs + L".VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    pixelShader_.Attach(Compile(L"Assets/Shaders/", _ps + L".PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
}

bool Shader::Create(const std::wstring& _name) {
    name_ = _name;

    CreateDxc();
    CompileShaders();

    return true;
}

bool Shader::Create(const std::wstring& _vs, const std::wstring& _ps) {
    CreateDxc();
    CompileShaders(_vs, _ps);

    return true;
}

Shader* Shader::PSLoad(const std::wstring& _name) {
    pixelShader_.Attach(Compile(L"Assets/Shaders/", _name + L".hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    return this;
}

IDxcBlob* Shader::Compile(const std::wstring& _directoryPath, const std::wstring& _filePath, const wchar_t* _profile, IDxcUtils* _dxcUtils, IDxcCompiler3* _dxcCompiler, IDxcIncludeHandler* _includeHandler) {
    Log::Send(Log::Level::INFO, Utils::Convert(std::format(L"Begin CompileShader, Path : {}, Profile : {}", _filePath, _profile)));
    HRESULT hResult = S_FALSE;
    IDxcBlobEncoding* shaderSource = nullptr;
    std::wstring fullPath = _directoryPath + _filePath;
    hResult = _dxcUtils->LoadFile(fullPath.c_str(), nullptr, &shaderSource);
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
        _filePath.c_str(), 
        L"-E", L"main", //EntryPoint
        L"-T", _profile, //ShaderProfile
        L"-I", _directoryPath.c_str(), //IncludePath
        L"-Zi", L"-Qembed_debug", //DebugInfo
        L"-Od", 
        L"-Zpr", 
    };

    IDxcResult* shaderResult = nullptr;
    hResult = _dxcCompiler->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        _includeHandler,
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

    Log::Send(Log::Level::INFO, Utils::Convert(std::format(L"Compile Succeed, Path : {}, Profile : {}", _filePath, _profile)));
    shaderSource->Release();
    shaderResult->Release();

    return shaderBlob;
}

