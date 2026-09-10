#include "SRVHandle.hpp"

#include <utility>

#include "SRVManager.h"

SRVHandle::SRVHandle(SRVManager* _owner, uint32_t _index) : owner_(_owner), index_(_index) {}

SRVHandle::SRVHandle(SRVHandle&& _other) noexcept {
    *this = std::move(_other);
}

SRVHandle& SRVHandle::operator=(SRVHandle&& _other) noexcept {
    if (this != &_other) {
        Reset();
        owner_ = _other.owner_;
        index_ = _other.index_;
        _other.owner_ = nullptr;
        _other.index_ = UINT32_MAX;
    }
    return *this;
}

SRVHandle::~SRVHandle() {
    Reset();
}

void SRVHandle::Reset() {
    if (owner_) {
        owner_->Free(index_);
    }
    owner_ = nullptr;
    index_ = UINT32_MAX;
}
