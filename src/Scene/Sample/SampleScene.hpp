#ifndef SampleScene_HPP_
#define SampleScene_HPP_
#include <memory>

#include "IScene.hpp"
#include "Model.hpp"

class SampleScene final : public IScene{
    std::unique_ptr<Model> model_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;
private:

}; // class SampleScene

#endif // SampleScene_HPP_
