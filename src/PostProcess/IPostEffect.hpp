#ifndef IPostEffect_HPP_
#define IPostEffect_HPP_

class IPostEffect {
protected:
    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;

    D3D12_GPU_DESCRIPTOR_HANDLE inputHandle_{};
    D3D12_CPU_DESCRIPTOR_HANDLE outputHandle_{};

    std::unique_ptr<DX12Resource> texture_;

    std::unique_ptr<DX12Resource> buffer_;


public:
    virtual ~IPostEffect() = default;
    virtual void Apply() = 0;
}; // class IPostEffect

#endif // IPostEffect_HPP_
