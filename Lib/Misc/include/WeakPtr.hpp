#ifndef WeakPtr_HPP_
#define WeakPtr_HPP_
#include <memory>
#include <stdexcept>

/** GameEngine Standard
 */
namespace GESTD {

    template<typename T>
    class WeakPtr {
        T* ptr_ = nullptr;

        void Validate() const {
            if (ptr_ == nullptr) {
                throw std::runtime_error("WeakPtr: null dereference");
            }
        }

    public:
        WeakPtr() = default;
        WeakPtr(std::nullptr_t) noexcept {}
        explicit WeakPtr(T* _ptr) noexcept : ptr_(_ptr) {}
        explicit WeakPtr(const std::unique_ptr<T>& _ptr) noexcept : ptr_(_ptr.get()) {}
        WeakPtr& operator=(std::nullptr_t) noexcept { ptr_ = nullptr; return *this; }
        WeakPtr& operator=(T* _ptr) noexcept { ptr_ = _ptr; return *this; }
        WeakPtr& operator=(const std::unique_ptr<T>& _ptr) { ptr_ = _ptr.get(); return *this; }

        T* operator->() const { Validate(); return ptr_; }
        T& operator*()  const { Validate(); return *ptr_; }
        operator T*() const noexcept { return ptr_; }

        explicit operator bool() const noexcept { return ptr_ != nullptr; }

        void Reset() noexcept { ptr_ = nullptr; }

    }; // class WeakPtr

} // namespace GESTD

#endif // WeakPtr_HPP_
