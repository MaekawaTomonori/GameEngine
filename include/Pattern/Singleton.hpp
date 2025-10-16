#ifndef Singleton_HPP_
#define Singleton_HPP_
#include <mutex>
#include <cassert>

/// <summary>
/// シングルトンファイナライザークラス
/// シングルトンインスタンスの破棄を管理
/// </summary>
class SingletonFinalizer{
    public:
    using Finalizer = void(*)();

    /// <summary>
    /// ファイナライザーを追加
    /// </summary>
    /// <param name="finalizer">終了処理関数</param>
    static void AddFinalizer(Finalizer finalizer);

    /// <summary>
    /// 全シングルトンを破棄
    /// </summary>
    static void Finalize();
};

/// <summary>
/// シングルトンパターンテンプレートクラス
/// スレッドセーフな単一インスタンス管理を提供
/// </summary>
template <typename T>
class Singleton {
    static T* instance_;
    static std::once_flag flag_;

public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    /// <summary>
    /// シングルトンインスタンスを取得
    /// </summary>
    /// <returns>インスタンスへのポインタ</returns>
    static T* GetInstance() {
        std::call_once(flag_, Create);
        assert(instance_);
        return instance_;
    }

private:
    Singleton() = default;
    ~Singleton() = default;

    static void Create() {
        instance_ = new T();
        SingletonFinalizer::AddFinalizer(Destroy);
    }
    static void Destroy() {
        delete instance_;
        instance_ = nullptr;
    }
};

template <typename T> inline T* Singleton<T>::instance_ = nullptr;
template <typename T> inline std::once_flag Singleton<T>::flag_;
#endif // Singleton_HPP_
