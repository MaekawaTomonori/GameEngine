#ifndef InputLayout_HPP_
#define InputLayout_HPP_
#include <d3d12.h>
#include <vector>

class InputLayout {
    D3D12_INPUT_LAYOUT_DESC desc_ {};
    std::vector<D3D12_INPUT_ELEMENT_DESC> elements_;

public:
    InputLayout& SetElement(const D3D12_INPUT_ELEMENT_DESC& _element);
    //InputLayout SetElement(InputElement _element);

    D3D12_INPUT_LAYOUT_DESC Get() const;
}; // class InputLayout

#endif // InputLayout_HPP_
