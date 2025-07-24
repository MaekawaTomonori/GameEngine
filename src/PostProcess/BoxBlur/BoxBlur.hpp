#ifndef BoxBlur_HPP_
#define BoxBlur_HPP_
#include "src/PostProcess/IPostEffect.hpp"

class BoxBlur : public IPostEffect{
    struct Material {
        Vector4 color;
    };

    std::unique_ptr<DX12Resource> mr_;
    Material* material_ = nullptr;
public:
    BoxBlur(DirectXAdapter* _adapter, SRVManager* _srv) :
        IPostEffect(_adapter, _srv) {
    }

    void Initialize() override;
    void Debug() override;

protected:
    void Modifier() override;

}; // class BoxBlur

#endif // BoxBlur_HPP_