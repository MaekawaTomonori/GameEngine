#ifndef ParticleSystem_HPP_
#define ParticleSystem_HPP_
#include <memory>
#include <string>
#include <unordered_map>

#include "Emitter/Emitter.hpp"
#include "Math/Vector3.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/Mesh/Repository/MeshRepository.hpp"
#include "src/Renderer/Renderer.hpp"

class ParticleSystem {
    struct Group {
        Vector3 position;
        std::vector<std::unique_ptr<Emitter>> emitters;

        explicit Group(const Vector3 _position) :
            position(_position) {
        }
    };

public:
    struct EmitterConfig {
        std::string texture = "white_x16.png";
        float frequency = 1.0f;
    };

    class GroupEditor {
        std::string name_;
        Group* group_;

        DirectXAdapter* adapter_ = nullptr;
        SRVManager* srv_ = nullptr;
        MeshRepository* mesh_ = nullptr;

    public:
        GroupEditor(std::string _name, Group* _group, DirectXAdapter* _adapter, SRVManager* _srv, MeshRepository* _mesh);
        GroupEditor& AddEmitter(const EmitterConfig& _config);
    };

private:
    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;
    MeshRepository* mesh_ = nullptr;

    std::unordered_map<std::string, std::unique_ptr<Group>> groups_;

    std::unique_ptr<PipelineStateObject> pso_ = nullptr;

public:
    ParticleSystem(DirectXAdapter* _adapter, SRVManager* _srv, MeshRepository* _mesh);

    void Initialize();
    void Update();
    void Draw(std::reference_wrapper<Renderer> _renderer);

    GroupEditor Register(const std::string& _name, Vector3 _position);
    GroupEditor Edit(const std::string& _name) const;
    void Delete(const std::string& _name);

    /// @brief Registers a new particle group from a JSON string.
    /// @param _json The JSON string containing the group data.
    /// @param _name The name of the group (optional).
    //void RegisterFromJson(const std::string& _json, const std::string& _name = "");
    //void ExportToJson(const std::string& _name) const;

private:
    void SetupPSO();
}; // class ParticleSystem

#endif // ParticleSystem_HPP_
