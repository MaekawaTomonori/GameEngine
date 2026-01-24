#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace spdlog {
    class logger;
    namespace sinks {
        class sink;
    }
}

/** @brief ロギングクラス
 ** spdlogを使用したログ出力とファイル管理を提供
 **/
class Log {
public:
    /** @brief ログレベル列挙型
     **/
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
    /** @brief ロギングシステムを初期化
     **/
    static void Initialize();

    /** @brief ログメッセージを送信
     ** @param _level ログレベル
     ** @param _message メッセージ
     **/
    static void Send(Level _level, const std::string& _message = "");

    /** @brief ログメッセージを送信(Debug)
     ** @param _message メッセージ
     **/
    static void Send(const std::string& _message);

    /** @brief コンテキスト付きログメッセージを送信
     ** @param _level ログレベル
     ** @param _message メッセージ
     ** @param _context コンテキスト情報
     **/
    static void SendWithContext(Level _level, const std::string& _message, const std::string& _context = "");

    /** @brief 実行コンテキストをログに記録
     **/
    static void LogExecutionContext();

    /** @brief ファイル操作をログに記録
     ** @param _operation 操作名
     ** @param _filePath ファイルパス
     ** @param _success 成功したかどうか
     ** @param _details 詳細情報
     **/
    static void LogFileOperation(const std::string& _operation, const std::string& _filePath, bool _success = true, const std::string& _details = "");

    /** @brief ログレベルを設定
     ** @param _level ログレベル
     **/
    static void SetLevel(Level _level);

private:
    /** @brief 実行コンテキストの初期化
     **/
    static void InitializeExecutionContext();

    /** @brief 現在の作業ディレクトリを取得
     ** @return 作業ディレクトリパス
     **/
    static std::string GetCurrentWorkingDirectory();

    /** @brief 実行ファイルのパスを取得
     ** @return 実行ファイルパス
     **/
    static std::string GetExecutablePath();

    /** @brief 作業ディレクトリをログに記録（レガシー互換性、削除予定）
     **/
    static void LogWorkingDirectory();

    /** @brief ファイルシステム診断情報をログに記録（レガシー互換性、削除予定）
     ** @param _targetPath 対象パス
     ** @param _context コンテキスト
     **/
    static void LogFileSystemDiagnostics(const std::string& _targetPath, const std::string& _context = "");

    /** @brief パス付きログメッセージを送信（レガシー互換性、削除予定）
     ** @param _level ログレベル
     ** @param _message メッセージ
     ** @param _context コンテキスト
     **/
    static void SendWithPath(Level _level, const std::string& _message, const std::string& _context = "");
};

#endif //LOG_HPP
