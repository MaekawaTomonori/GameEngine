#ifndef Singleton_HPP_
#define Singleton_HPP_
#include <mutex>
#include <cassert>

class SingletonFinalizer{
	public:
	using Finalizer = void(*)();
	static void AddFinalizer(Finalizer finalizer);
	static void Finalize();
};

template <typename T>
class Singleton {
	static T* instance_;
	static std::once_flag flag_;
public:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

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
