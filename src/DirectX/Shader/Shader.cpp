#include "Shader.h"

#include <cassert>
#include <format>
#include <filesystem>

#include "Log.hpp"
#include "Utils.hpp"

Shader::Shader(const std::wstring& _name) {
    if (!Create(_name)) {
        Log::SendWithContext(Log::Level::ERR, Utils::Convert(std::format(L"Failed to create shader: {}", _name)), "SHADER");
        Utils::Alert("Failed to create shader: " + Utils::Convert(_name));
        return;
    }
    Log::SendWithContext(Log::Level::INFO, Utils::Convert(std::format(L"Shader created successfully: {}", _name)), "SHADER");
}

Shader::Shader(const std::wstring& _vs, const std::wstring& _ps) {
    if (!Create(_vs, _ps)){
        Log::SendWithContext(Log::Level::ERR, Utils::Convert(std::format(L"Failed to create shader: {}, {}", _vs, _ps)), "SHADER");
        Utils::Alert("Failed to create shader: " + Utils::Convert(_vs) + ", " + Utils::Convert(_ps));
        return;
    }
    Log::SendWithContext(Log::Level::INFO, Utils::Convert(std::format(L"Shader created successfully: {}, {}", _vs, _ps)), "SHADER");
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
    std::wstring shaderDir = FindShaderDirectory();
    vertexShader_.Attach(Compile(shaderDir, name_ + L".VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    pixelShader_.Attach(Compile(shaderDir, name_ + L".PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));

    /*if (name_ == L"Particle"){
        geometryShader_.Attach(Compile(L"Assets/Shaders/", name_ + L".GS.hlsl", L"gs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    }*/
}

void Shader::CompileShaders(const std::wstring& _vs, const std::wstring& _ps) {
    std::wstring shaderDir = FindShaderDirectory();
    vertexShader_.Attach(Compile(shaderDir, _vs + L".VS.hlsl", L"vs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    pixelShader_.Attach(Compile(shaderDir, _ps + L".PS.hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
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
    std::wstring shaderDir = FindShaderDirectory();
    pixelShader_.Attach(Compile(shaderDir, _name + L".hlsl", L"ps_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    return this;
}

IDxcBlob* Shader::CompileCS(const std::wstring& _name) const {
    std::wstring shaderDir = FindShaderDirectory();
    Microsoft::WRL::ComPtr<IDxcBlob> computeShader;
    computeShader.Attach(Compile(shaderDir, _name + L".CS.hlsl", L"cs_6_0", dxcUtils_.Get(), dxcCompiler_.Get(), includeHandler_.Get()));
    return computeShader.Get();
}

IDxcBlob* Shader::Compile(const std::wstring& _directoryPath, const std::wstring& _filePath, const wchar_t* _profile, IDxcUtils* _dxcUtils, IDxcCompiler3* _dxcCompiler, IDxcIncludeHandler* _includeHandler) {
    Log::SendWithContext(Log::Level::INFO, Utils::Convert(std::format(L"Begin CompileShader, Path : {}, Profile : {}", _filePath, _profile)), "SHADER");

    HRESULT hResult = S_FALSE;
    IDxcBlobEncoding* shaderSource = nullptr;
    std::wstring fullPath = _directoryPath + _filePath;

    // Log file operation attempt
    Log::LogFileOperation("SHADER_LOAD", Utils::Convert(fullPath), true, "Attempting to load shader file");

    hResult = _dxcUtils->LoadFile(fullPath.c_str(), nullptr, &shaderSource);
    if (FAILED(hResult)) {
        Log::LogFileOperation("SHADER_LOAD", Utils::Convert(fullPath), false, "DirectX shader file load failed");
        Utils::Alert("Failed to load shader file: " + Utils::Convert(fullPath));
        return nullptr;
    }

    Log::LogFileOperation("SHADER_LOAD", Utils::Convert(fullPath), true, "Shader file loaded successfully");

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

    Log::SendWithContext(Log::Level::INFO, Utils::Convert(std::format(L"Compile Succeed, Path : {}, Profile : {}", _filePath, _profile)), "SHADER");
    shaderSource->Release();
    shaderResult->Release();

    return shaderBlob;
}

std::wstring Shader::FindShaderDirectory() {
    Log::SendWithContext(Log::Level::INFO, "Searching for shader directory", "SHADER_PATH");

    // List of possible shader directory paths to try
    std::vector<std::wstring> candidatePaths = {
        L"Assets/Shaders/",                    // Original relative path
        L"./Assets/Shaders/",                  // Explicit current directory
        L"../Assets/Shaders/",                 // Parent directory
        L"../../Assets/Shaders/",              // Grandparent directory
        L"../Engine/Assets/Shaders/",          // Relative to Game project
        L"../../Engine/Assets/Shaders/",       // Two levels up from Game output
        L"Engine/Assets/Shaders/",             // Direct Engine path
        L"GameTemplate/Engine/Assets/Shaders/" // Full relative path from solution
    };

    for (const auto& candidatePath : candidatePaths) {
        try {
            std::string candidatePathStr = Utils::Convert(candidatePath);
            if (std::filesystem::exists(candidatePath)) {
                Log::LogFileOperation("DIRECTORY_CHECK", candidatePathStr, true, "Directory exists");

                // Verify it contains shader files
                bool hasShaderFiles = false;
                for (const auto& entry : std::filesystem::directory_iterator(candidatePath)) {
                    if (entry.path().extension() == L".hlsl") {
                        hasShaderFiles = true;
                        break;
                    }
                }

                if (hasShaderFiles) {
                    Log::LogFileOperation("SHADER_DIRECTORY", candidatePathStr, true, "Valid shader directory found with .hlsl files");
                    return candidatePath;
                } else {
                    Log::LogFileOperation("SHADER_DIRECTORY", candidatePathStr, false, "Directory exists but contains no .hlsl files");
                }
            } else {
                Log::LogFileOperation("DIRECTORY_CHECK", candidatePathStr, false, "Directory does not exist");
            }
        } catch (const std::exception& _e) {
            Log::LogFileOperation("DIRECTORY_CHECK", Utils::Convert(candidatePath), false, std::string("Exception: ") + e.what());
        }
    }

    // If no valid path found, log error and return default
    Log::SendWithContext(Log::Level::ERR, "No valid shader directory found! Using default path.", "SHADER_PATH");
    Log::LogExecutionContext();

    return L"Assets/Shaders/";
}
