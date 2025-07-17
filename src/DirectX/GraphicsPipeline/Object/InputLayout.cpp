#include "InputLayout.hpp"

InputLayout InputLayout::SetElement(const D3D12_INPUT_ELEMENT_DESC& _element) {
    elements_.push_back(_element);
    return *this;
}

D3D12_INPUT_LAYOUT_DESC InputLayout::Get() const {
    return desc_;
}

//InputLayout InputLayout::SetElement(InputElement _element) {
//    return *this;
//}
