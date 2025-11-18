#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

// Forward declarations
namespace spdlog {
    class logger;
    namespace sinks {
        class sink;
    }
}

/// <summary>
/// ロギングクラス
/// spdlogを使用したログ出力とファイル管理を提供
/// </summary>
class Log {
public:
    /// <summary>
    /// ログレベル列挙型
    /// </summary>
    enum class Level{
        TRACE = 0,
        DBG = 1,
        INFO = 2,
        WARNING = 3,
        ERR = 4,
        FATAL = 5,
    };

private:
    static std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks_;
    static Level level_;
    static std::string logFilePath_;
    static std::string logFileName_;
    static std::string logFileExt_;
    static std::string executablePath_;
    static std::string workingDirectory_;

public:
    /// <summary>
    /// ロギングシステムを初期化
    /// </summary>
    static void Initialize();

    /// <summary>
    /// ログメッセージを送信
    /// </summary>
    /// <param name="level">ログレベル</param>
    /// <param name="message">メッセージ</param>
    static void Send(Level level, const std::string& message = "");

    /// <summary>
    /// ログメッセージを送信(Debug)
    /// </summary>
    /// <param name="message">メッセージ</param>
    static void Send(const std::string& message);

    /// <summary>
    /// コンテキスト付きログメッセージを送信
    /// </summary>
    /// <param name="level">ログレベル</param>
    /// <param name="message">メッセージ</param>
    /// <param name="context">コンテキスト情報</param>
    static void SendWithContext(Level level, const std::string& message, const std::string& context = "");

    /// <summary>
    /// 実行コンテキストをログに記録
    /// </summary>
    static void LogExecutionContext();

    /// <summary>
    /// ファイル操作をログに記録
    /// </summary>
    /// <param name="operation">操作名</param>
    /// <param name="filePath">ファイルパス</param>
    /// <param name="success">成功したかどうか</param>
    /// <param name="details">詳細情報</param>
    static void LogFileOperation(const std::string& operation, const std::string& filePath, bool success = true, const std::string& details = "");

    /// <summary>
    /// ログレベルを設定
    /// </summary>
    /// <param name="level">ログレベル</param>
    static void SetLevel(Level level);

private:
    /// <summary>
    /// 実行コンテキストの初期化
    /// </summary>
    static void InitializeExecutionContext();

    /// <summary>
    /// 現在の作業ディレクトリを取得
    /// </summary>
    /// <returns>作業ディレクトリパス</returns>
    static std::string GetCurrentWorkingDirectory();

    /// <summary>
    /// 実行ファイルのパスを取得
    /// </summary>
    /// <returns>実行ファイルパス</returns>
    static std::string GetExecutablePath();

    /// <summary>
    /// 作業ディレクトリをログに記録（レガシー互換性、削除予定）
    /// </summary>
    static void LogWorkingDirectory();

    /// <summary>
    /// ファイルシステム診断情報をログに記録（レガシー互換性、削除予定）
    /// </summary>
    /// <param name="targetPath">対象パス</param>
    /// <param name="context">コンテキスト</param>
    static void LogFileSystemDiagnostics(const std::string& targetPath, const std::string& context = "");

    /// <summary>
    /// パス付きログメッセージを送信（レガシー互換性、削除予定）
    /// </summary>
    /// <param name="level">ログレベル</param>
    /// <param name="message">メッセージ</param>
    /// <param name="context">コンテキスト</param>
    static void SendWithPath(Level level, const std::string& message, const std::string& context = "");
};

#endif //LOG_HPP
