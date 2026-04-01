#ifndef WeakPtr_HPP_
#define WeakPtr_HPP_
#include <memory>
#include <stdexcept>

/** GameEngine Standard
 */
namespace GESTD {

    template<typename T>
    class ReferencePtr {
        T* ptr_ = nullptr;

        void Validate() const {
            if (ptr_ == nullptr) {
                throw std::runtime_error("ReferencePtr: null dereference");
            }
        }

    public:
        ReferencePtr() = default;
        ReferencePtr(std::nullptr_t) noexcept {}
        explicit ReferencePtr(T* _ptr) noexcept : ptr_(_ptr) {}
        explicit ReferencePtr(const std::unique_ptr<T>& _ptr) noexcept : ptr_(_ptr.get()) {}
        ReferencePtr& operator=(std::nullptr_t) noexcept { ptr_ = nullptr; return *this; }
        ReferencePtr& operator=(T* _ptr) noexcept { ptr_ = _ptr; return *this; }
        ReferencePtr& operator=(const std::unique_ptr<T>& _ptr) { ptr_ = _ptr.get(); return *this; }

        T* operator->() const { Validate(); return ptr_; }
        T& operator*()  const { Validate(); return *ptr_; }

        // @Deprecated Methods
        operator T*() const noexcept { return ptr_; }

        explicit operator bool() const noexcept { return ptr_ != nullptr; }

        void Reset() noexcept { ptr_ = nullptr; }

    }; // class ReferencePtr

} // namespace GESTD

#endif // WeakPtr_HPP_
