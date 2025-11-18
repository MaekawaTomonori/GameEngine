#ifndef ParticleSystem_HPP_
#define ParticleSystem_HPP_
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "Emitter/Emitter.hpp"
#include "Math/Vector3.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/Renderer/Renderer.hpp"

class ParticleSystem {
    struct Group {
        Vector3 position;
        std::vector<std::unique_ptr<Emitter>> emitters;

        Group(Vector3 _position) : position(_position) {}
    };

public:
    class GroupEditor {
        Group* group_;
    public:
        GroupEditor(Group* _group);
        GroupEditor& AddEmitter();
    };

private:
    std::optional<std::reference_wrapper<DirectXAdapter>> adapter_;
    std::optional<std::reference_wrapper<SRVManager>> srv_;

    std::unordered_map<std::string, std::unique_ptr<Group>> groups_;

public:
    ParticleSystem(DirectXAdapter& _adapter, SRVManager& _srv);

    void Initialize();
    void Update();
    void Draw(std::reference_wrapper<Renderer> _renderer);

    GroupEditor Register(const std::string& _name, Vector3 _position);
    GroupEditor Edit(const std::string& _name) const;
    void Delete(const std::string& _name);

private:

}; // class ParticleSystem

#endif // ParticleSystem_HPP_
