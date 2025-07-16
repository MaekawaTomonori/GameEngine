#ifndef IPostEffect_HPP_
#define IPostEffect_HPP_

class IPostEffect {
public:
    virtual ~IPostEffect() = default;
    virtual void Apply() = 0;
}; // class IPostEffect

#endif // IPostEffect_HPP_
