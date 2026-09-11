#ifndef ModelTypeRenderer_HPP_
#define ModelTypeRenderer_HPP_
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/Renderer/Renderer.hpp"

/** @brief モデル種別ごとの描画キューとPSOをまとめて管理するテンプレート
 * 種別ごとにExecuteDraw()を非virtualに直接呼び出すため、std::functionによる型消去を経由しない。
 * 新しいモデル種別を追加する際は、このテンプレートを1つインスタンス化するだけで済む。
 */
template <typename T>
class ModelTypeRenderer {
    struct Entry {
        T* instance;
        std::string canvasName;
    };

    std::vector<Entry> opaque_;
    std::vector<Entry> transparent_;

public:
    std::unique_ptr<PipelineStateObject> pipeline;
    std::unique_ptr<PipelineStateObject> transparentPipeline;

    void Register(T* _instance, bool _isTransparent, const std::string& _canvasName) {
        (_isTransparent ? transparent_ : opaque_).push_back({ _instance, _canvasName });
    }

    void Flush(Renderer* _renderer, const std::function<void()>& _bindShadow) {
        FlushQueue(opaque_, pipeline.get(), _renderer, _bindShadow);
        FlushQueue(transparent_, transparentPipeline.get(), _renderer, _bindShadow);
    }

private:
    static void FlushQueue(std::vector<Entry>& _queue, PipelineStateObject* _pipeline, Renderer* _renderer, const std::function<void()>& _bindShadow) {
        if (_queue.empty()) return;

        std::unordered_map<std::string, std::vector<T*>> groups;
        for (const auto& entry : _queue) {
            groups[entry.canvasName].push_back(entry.instance);
        }
        _queue.clear();

        for (auto& [canvasName, instances] : groups) {
            _renderer->Register([_pipeline, instances = std::move(instances), _bindShadow]() {
                if (_pipeline) {
                    _pipeline->DrawCall();
                }
                _bindShadow();
                for (T* instance : instances) {
                    instance->ExecuteDraw();
                }
            }, canvasName);
        }
    }
}; // class ModelTypeRenderer

#endif // ModelTypeRenderer_HPP_
