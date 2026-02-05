#include "InputLayout.hpp"

InputLayout& InputLayout::SetElement(const D3D12_INPUT_ELEMENT_DESC& _element) {
    elements_.push_back(_element);
    return *this;
}

D3D12_INPUT_LAYOUT_DESC InputLayout::Get() const {
    D3D12_INPUT_LAYOUT_DESC result = desc_;
    result.pInputElementDescs = elements_.data();
    result.NumElements = static_cast<UINT>(elements_.size());
    return result;
}
