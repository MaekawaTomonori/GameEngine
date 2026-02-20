#ifndef ParticleSystem_HPP_
#define ParticleSystem_HPP_
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Emitter/Emitter.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/Mesh/Repository/MeshRepository.hpp"
#include "src/Renderer/Renderer.hpp"

class ParticleSystem {
public:
    using UpdateFunc = std::function<void(float, const Vector3&, Vector3&, Vector3&, Vector4&)>;
    using SpawnFunc  = std::function<void(const Vector3&, Vector3&, Vector3&)>;

    struct EmitterConfig {
        std::string texture = "white_x16.png";
        float frequency = 0.f;
        float duration = 1.f;
        uint16_t spawnCount = 1;
        Vector3 size = {1.f, 1.f, 1.f};
        Vector3 velocity = {0.f, 0.f, 0.f};
        Vector4 color = {1.f, 1.f, 1.f, 1.f};
        std::string updateFuncKey;
        UpdateFunc updateFunc;
        std::string spawnFuncKey;
        SpawnFunc spawnFunc;
    };

    struct Template {
        std::vector<EmitterConfig> emitters;
    };

    class TemplateEditor {
        Template* template_ = nullptr;

    public:
        explicit TemplateEditor(Template* _template);
        TemplateEditor& AddEmitter(const EmitterConfig& _config);
    };

private:
    static constexpr uint16_t POOL_SIZE = 64;

    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;
    MeshRepository* mesh_ = nullptr;
    DebugUI* debugUI_ = nullptr;

    std::unordered_map<std::string, Template> templates_;
    std::unordered_map<std::string, UpdateFunc> updateFuncs_;
    std::unordered_map<std::string, SpawnFunc>  spawnFuncs_;

    std::vector<std::unique_ptr<Emitter>> pool_;
    std::vector<Emitter*> available_;
    std::vector<Emitter*> active_;

    bool poolInitialized_ = false;
    std::unique_ptr<PipelineStateObject> pso_ = nullptr;

public:
    ParticleSystem(DirectXAdapter* _adapter, SRVManager* _srv, MeshRepository* _mesh, DebugUI* _debugUI);

    void Initialize();
    void Update();
    void Draw(Renderer* _renderer);

    /** @brief 更新関数を文字列キーで登録
     ** @param _key 関数を識別するキー
     ** @param _func 更新関数
     **/
    void RegisterUpdateFunc(const std::string& _key, UpdateFunc _func);

    /** @brief スポーン関数を文字列キーで登録
     ** @param _key 関数を識別するキー
     ** @param _func スポーン関数 (emitterCenter, outPos, outVel)
     **/
    void RegisterSpawnFunc(const std::string& _key, SpawnFunc _func);

    /** @brief テンプレートを登録（空のテンプレートを作成しEditorを返す）
     ** @param _name テンプレート名
     ** @return TemplateEditor
     **/
    TemplateEditor Register(const std::string& _name);

    /** @brief テンプレートを登録（既存のTemplateをコピー）
     ** @param _name テンプレート名
     ** @param _template テンプレートデータ
     ** @return TemplateEditor
     **/
    TemplateEditor Register(const std::string& _name, const Template& _template);

    /** @brief JSONファイルからテンプレートを読み込み
     ** @param _name ファイル名（Assets/Data/Particle/<name>.json）
     **/
    void LoadTemplate(const std::string& _name);

    /** @brief テンプレートをJSONファイルに保存
     ** @param _name テンプレート名
     **/
    void SaveTemplate(const std::string& _name) const;

    /** @brief テンプレートからInstanceを生成してパーティクルを発射
     ** @param _templateName テンプレート名
     ** @param _position 発射位置
     **/
    void Emit(const std::string& _templateName, const Vector3& _position);

    /** @brief テンプレートを削除
     ** @param _name テンプレート名
     **/
    void DeleteTemplate(const std::string& _name);


private:
    void SetupPSO();
    void InitializePool();
    void Debug();
    UpdateFunc ResolveUpdateFunc(const EmitterConfig& _config) const;
    SpawnFunc  ResolveSpawnFunc(const EmitterConfig& _config) const;
}; // class ParticleSystem

#endif // ParticleSystem_HPP_
