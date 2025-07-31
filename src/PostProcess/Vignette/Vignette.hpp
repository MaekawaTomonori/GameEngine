#ifndef Vignette_HPP_
#define Vignette_HPP_
#include "src/PostProcess/IPostEffect.hpp"

class Vignette : public IPostEffect{
    struct Material {
        Vector4 color;
        float intensity;
        float scale;
        float pad[2];
    };

    std::unique_ptr<DX12Resource> mr_;
    Material* material_ = nullptr;

public:
    Vignette(DirectXAdapter* _adapter, SRVManager* _srv) :
        IPostEffect(_adapter, _srv) {
    }

    void Initialize() override;
    void Debug() override;

protected:
    void Modifier() override;

}; // class Vignette

#endif // Vignette_HPP_