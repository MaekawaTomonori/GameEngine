#ifndef SampleScene_HPP_
#define SampleScene_HPP_
#include <memory>

#include "IScene.hpp"
#include "Model.hpp"
#include "Handle.hpp"

class SampleScene final : public IScene{
    std::unique_ptr<Model> model_;
    std::unique_ptr<Model> plane;

    Audio::Handle kick_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;
private:

}; // class SampleScene

#endif // SampleScene_HPP_
