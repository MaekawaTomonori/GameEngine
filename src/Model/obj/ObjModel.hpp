#ifndef ObjModel_HPP_
#define ObjModel_HPP_
#include "Model.hpp"

class ObjModel final : public Model{
    ObjData data_;
public:
    void Initialize(const std::string& _name) override;
    void Update() override;
    void Draw() const override;

private:
    void Debug() override;
}; // class ObjModel

#endif // ObjModel_HPP_
