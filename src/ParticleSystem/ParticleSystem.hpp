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
        bool enable = false;
        Vector3 position;
        std::vector<std::unique_ptr<Emitter>> emitters;

        explicit Group(const Vector3 _position) :
            position(_position) {
        }
    };

public:
    struct EmitterConfig {
        std::string texture = "white_x16.png";
        bool active = false;
        float frequency = 1.0f;
        float duration = 1.f;
        uint16_t spawnCount = 1;
        Vector3 size = {1.f, 1.f, 1.f};
        Vector3 velocity = {0.f, 0.f, 0.f};
        Vector4 color = { 1.f, 1.f, 1.f, 1.f };
        // float time, Vector3& current velocity, Vector4& current color
        std::function<void(float, Vector3&, Vector4&)> updateFunc;
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
        GroupEditor& SetPosition(const Vector3& _position);
        GroupEditor& SetEnable(bool _enable);
        GroupEditor& ClearEmitters();
        GroupEditor& Emit();
    };

private:
    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;
    MeshRepository* mesh_ = nullptr;
    DebugUI* debugUI_ = nullptr;

    std::unordered_map<std::string, std::unique_ptr<Group>> groups_;

    std::unique_ptr<PipelineStateObject> pso_ = nullptr;

public:
    ParticleSystem(DirectXAdapter* _adapter, SRVManager* _srv, MeshRepository* _mesh, DebugUI* _debugUI);

    void Initialize();
    void Update();
    void Draw(Renderer* _renderer);

    GroupEditor Register(const std::string& _name, Vector3 _position = Vector3{ 0.f, 0.f, 0.f });
    GroupEditor Edit(const std::string& _name) const;
    void Enable(const std::string& _name) const;
    void Delete(const std::string& _name);

    void Emit(const std::string& _name);

    /// @brief Registers a new particle group from a JSON string.
    /// @param _json The JSON string containing the group data.
    /// @param _name The name of the group (optional).
    //void RegisterFromJson(const std::string& _json, const std::string& _name = "");
    //void ExportToJson(const std::string& _name) const;

private:
    void SetupPSO();
    void Debug();
}; // class ParticleSystem

#endif // ParticleSystem_HPP_
