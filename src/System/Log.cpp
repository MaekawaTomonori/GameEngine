#include "include/Log.hpp"

#include "src/DirectX/DirectXAdapter.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <format>

#include "imgui_internal.h"

#ifdef _WIN32
#include <spdlog/sinks/msvc_sink.h>
#include <Windows.h>
#endif

#undef max
#undef min

// Static member definitions
std::vector<std::shared_ptr<spdlog::sinks::sink>> Log::sinks_;
Log::Level Log::level_ = Level::INFO;
std::string Log::logFilePath_ = "Log/";
std::string Log::logFileName_ = "LatestLog";
std::string Log::logFileExt_ = ".log";
std::string Log::executablePath_;
std::string Log::workingDirectory_;
std::deque<Log::LogEntry> Log::entries_;
std::mutex                Log::entriesMutex_;
std::array<char, 256>     Log::buffer_{};
int                       Log::panelLevel_ = static_cast<int>(Log::Level::INFO);

namespace {
    float FilterWidth = 90.f;
    bool  SettingsHandlerRegistered = false;

    const char* GetLevelLabel(Log::Level _level) {
        switch (_level) {
            case Log::Level::TRACE:   return "TRACE";
            case Log::Level::DBG:     return "DEBUG";
            case Log::Level::INFO:    return "INFO ";
            case Log::Level::WARNING: return "WARN ";
            case Log::Level::ERR:     return "ERROR";
            case Log::Level::FATAL:   return "FATAL";
            default:                  return "?????";
        }
    }

    ImVec4 GetLevelColor(Log::Level _level) {
        switch (_level) {
            case Log::Level::TRACE:   return {0.50f, 0.50f, 0.50f, 1.0f};
            case Log::Level::DBG:     return {0.40f, 0.80f, 1.00f, 1.0f};
            case Log::Level::INFO:    return {0.90f, 0.90f, 0.90f, 1.0f};
            case Log::Level::WARNING: return {1.00f, 0.80f, 0.20f, 1.0f};
            case Log::Level::ERR:     return {1.00f, 0.40f, 0.40f, 1.0f};
            case Log::Level::FATAL:   return {1.00f, 0.20f, 0.60f, 1.0f};
            default:                  return {1.00f, 1.00f, 1.00f, 1.0f};
        }
    }
} // namespace

void Log::Initialize() {
    // Initialize execution context information
    InitializeExecutionContext();

    // Clear any existing sinks
    sinks_.clear();

    // Console sink with execution context in pattern
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console->set_pattern("%^[%T] [%l] %v%$");
    sinks_.emplace_back(console);

    // Daily rotating file sink with full execution context
    auto file = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFilePath_ + logFileName_ + logFileExt_, 0, 0);
    file->set_pattern("[%Y-%m-%d %T] [%l] %v");
    sinks_.emplace_back(file);

#ifdef _WIN32
    // Visual Studio output window sink with execution context
    auto windows_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    windows_sink->set_pattern("[%T] [%l] %v");
    sinks_.emplace_back(windows_sink);
#endif

    // Create logger with all sinks
    auto logger = std::make_shared<spdlog::logger>("Engine", begin(sinks_), end(sinks_));
    logger->set_level(static_cast<spdlog::level::level_enum>(static_cast<int>(level_)));

    // Register as default logger
    spdlog::set_default_logger(logger);

    // Set up async logging
    spdlog::flush_every(std::chrono::seconds(1));

    spdlog::info("Log system initialized");

    // Log execution context immediately after initialization
    LogExecutionContext();
}

void Log::Send(Level _level, const std::string& _message) {
    switch (_level) {
        case Level::TRACE:
            spdlog::trace(_message);
            break;
        case Level::DBG:
            spdlog::debug(_message);
            break;
        case Level::INFO:
            spdlog::info(_message);
            break;
        case Level::WARNING:
            spdlog::warn(_message);
            break;
        case Level::ERR:
            spdlog::error(_message);
            break;
        case Level::FATAL:
            spdlog::critical(_message);
            break;
    }

    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tmVal{};
    localtime_s(&tmVal, &t);
    char buf[10];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tmVal);

    std::lock_guard lock(entriesMutex_);
    entries_.push_back({_level, _message, buf});
    if (entries_.size() > kMaxEntries) {
        entries_.pop_front();
    }
}

void Log::Send(const std::string& _message) {
    Send(Level::DBG, _message);
}

void Log::InitializeExecutionContext() {
    try {
        workingDirectory_ = GetCurrentWorkingDirectory();
        executablePath_ = GetExecutablePath();
    } catch (const std::exception& _e) {
        workingDirectory_ = "Error: " + std::string(_e.what());
        executablePath_ = "Error: " + std::string(_e.what());
    }
}

std::string Log::GetCurrentWorkingDirectory() {
    try {
        return std::filesystem::current_path().string();
    } catch (const std::exception& _e) {
        return "Error getting CWD: " + std::string(_e.what());
    }
}

std::string Log::GetExecutablePath() {
    try {
#ifdef _WIN32
        char exePath[MAX_PATH];
        DWORD result = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        if (result > 0) {
            return std::string(exePath);
        }
        return "Error getting executable path";
#else
        return "Non-Windows platform";
#endif
    } catch (const std::exception& _e) {
        return "Error: " + std::string(_e.what());
    }
}

void Log::SendWithContext(Level _level, const std::string& _message, const std::string& _context) {
    std::string contextPrefix = _context.empty() ? "" : "[" + _context + "] ";
    std::string enhancedMessage = contextPrefix + _message;
    Send(_level, enhancedMessage);
}

void Log::LogExecutionContext() {
    Send(Level::INFO, "=== Execution Context Information ===");
    Send(Level::INFO, std::format("Executable: {}", executablePath_));
    Send(Level::INFO, std::format("Working Directory: {}", workingDirectory_));
    Send(Level::INFO, "=== End Context Information ===");
}

void Log::LogFileOperation(const std::string& _operation, const std::string& _filePath, bool _success, const std::string& _details) {
    std::string status = _success ? "SUCCESS" : "FAILED";
    std::string message = std::format("FILE_OP [{}] {} -> {}", status, _operation, _filePath);

    if (!_details.empty()) {
        message += " (" + _details + ")";
    }

    // Check if file exists and add absolute path info
    try {
        if (std::filesystem::exists(_filePath)) {
            std::string absolutePath = std::filesystem::absolute(_filePath).string();
            message += std::format(" [ABS: {}]", absolutePath);
        } else {
            message += " [FILE_NOT_FOUND]";
        }
    } catch (const std::exception& _e) {
        message += std::format(" [PATH_ERROR: {}]", _e.what());
    }

    Level logLevel = _success ? Level::INFO : Level::ERR;
    Send(logLevel, message);
}

// Legacy compatibility functions
void Log::SendWithPath(Level _level, const std::string& _message, const std::string& _context) {
    SendWithContext(_level, _message, _context);
}

bool Log::SendFromPanel() {
    constexpr const char* LABELS[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

    // プレビュー部分だけ選択中レベルの色を適用し、ドロップダウン内は各自の色にする
    ImGui::SetNextItemWidth(70.f);
    ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(static_cast<Level>(panelLevel_)));
    const bool comboOpen = ImGui::BeginCombo("##panel_level", LABELS[panelLevel_]);
    ImGui::PopStyleColor(); // ここで解除してドロップダウン内に色が漏れないようにする

    if (comboOpen) {
        for (int i = 0; i < 6; i++) {
            ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(static_cast<Level>(i)));
            if (ImGui::Selectable(LABELS[i], panelLevel_ == i)) {
                panelLevel_ = i;
            }
            ImGui::PopStyleColor();
            if (panelLevel_ == i) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    constexpr float kSendBtnWidth = 46.f;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - kSendBtnWidth - ImGui::GetStyle().ItemSpacing.x);
    const bool enterPressed = ImGui::InputText("##panel_input", buffer_.data(), buffer_.size(),
        ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();
    const bool btnClicked = ImGui::Button("Send", ImVec2(kSendBtnWidth, 0));

    if ((enterPressed || btnClicked) && buffer_[0] != '\0') {
        Send(static_cast<Level>(panelLevel_), buffer_.data());
        return true;
    }
    return false;
}

void Log::LogWorkingDirectory() {
    LogExecutionContext();
}

void Log::LogFileSystemDiagnostics(const std::string& _targetPath, const std::string& _context) {
    std::string prefix = _context.empty() ? "" : "[" + _context + "] ";
    LogFileOperation("DIAGNOSTIC", _targetPath, std::filesystem::exists(_targetPath), prefix + "File system check");
}


void Log::SetLevel(Level _level) {
    level_ = _level;
    if (spdlog::get("Engine")) {
        spdlog::get("Engine")->set_level(static_cast<spdlog::level::level_enum>(static_cast<int>(_level)));
    }
}

void Log::Debug(DebugUI* _debug) {
#ifdef _DEBUG
    if (!SettingsHandlerRegistered) {
        SettingsHandlerRegistered = true;
        ImGuiSettingsHandler handler{};
        handler.TypeName = "LoggerFilter";
        handler.TypeHash = ImHashStr("LoggerFilter");
        handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*, const char*) -> void* {
            return reinterpret_cast<void*>(1);
        };
        handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler*, void*, const char* line) {
            float val;
            if (sscanf_s(line, "FilterWidth=%f", &val) == 1) {
                FilterWidth = std::clamp(val, 60.f, 300.f);
            }
        };
        handler.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* h, ImGuiTextBuffer* buf) {
            buf->appendf("[%s][Data]\n", h->TypeName);
            buf->appendf("FilterWidth=%f\n", FilterWidth);
        };
        ImGui::AddSettingsHandler(&handler);
    }
#endif

    _debug->RegisterMenuButton("Logger", true);
    _debug->RegisterCommand("Logger",
        [_debug] {
            ImGui::Begin("Logger", &_debug->IsVisible("Logger"));

            static bool  filters[6]  = {true, true, true, true, true, true};
            static bool  autoScroll  = false;

            // --- 2カラムレイアウト ---
            const float panelHeight = std::max(1.f, ImGui::GetContentRegionAvail().y);

            // 左: フィルター＋コントロールパネル
            ImGui::BeginChild("##filter_panel", ImVec2(FilterWidth, panelHeight), true);

            ImGui::TextDisabled("Filter");
            ImGui::Separator();
            for (int i = 0; i < 6; i++) {
                constexpr const char* LABELS[6] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
                ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(static_cast<Log::Level>(i)));
                ImGui::Checkbox(LABELS[i], &filters[i]);
                ImGui::PopStyleColor();
            }

            ImGui::Separator();
            ImGui::TextDisabled("Controls");
            ImGui::Separator();
            if (ImGui::Button("Clear")) {
                std::lock_guard lock(Log::entriesMutex_);
                Log::entries_.clear();
            }
            ImGui::Checkbox("Auto-scroll", &autoScroll);

            ImGui::EndChild();

            ImGui::SameLine();

            // ドラッグで幅変更できる仕切り
            ImGui::InvisibleButton("##splitter", ImVec2(4.f, panelHeight));
            if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }
            if (ImGui::IsItemActive()) {
                FilterWidth += ImGui::GetIO().MouseDelta.x;
                FilterWidth = std::max(FilterWidth, 60.f);
                FilterWidth = std::min(FilterWidth, 300.f);
            }

            ImGui::SameLine();

            // 右: ログ＋入力エリア（ラッパー）
            ImGui::BeginChild("##right_col", ImVec2(0, panelHeight), false);

            // 入力行の高さ分だけ残してログスクロールを縮める（負値 = 残り高さ - abs値）
            const float inputRowHeight = ImGui::GetFrameHeightWithSpacing();
            ImGui::BeginChild("##log_scroll", ImVec2(0, -inputRowHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

            std::vector<Log::LogEntry> snapshot;
            {
                std::lock_guard lock(Log::entriesMutex_);
                snapshot = {Log::entries_.begin(), Log::entries_.end()};
            }

            for (const auto& entry : snapshot) {
                const int idx = static_cast<int>(entry.level);
                if (idx >= 0 && idx < 6 && !filters[idx]) continue;

                ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(entry.level));
                ImGui::TextUnformatted(
                    std::format("[{}] [{}]  {}", entry.timestamp, GetLevelLabel(entry.level), entry.message).c_str()
                );
                ImGui::PopStyleColor();
            }

            if (autoScroll) {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild(); // ##log_scroll

            // 入力行は常にログ下部に固定表示
            if (SendFromPanel()) {
                std::ranges::fill(buffer_.begin(), buffer_.end(), '\0');
            }

            ImGui::EndChild(); // ##right_col

            ImGui::End();
        }
    );
}
