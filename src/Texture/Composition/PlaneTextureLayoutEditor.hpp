#ifndef PlaneTextureLayoutEditor_HPP_
#define PlaneTextureLayoutEditor_HPP_
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "ReferencePtr.hpp"
#include "src/Texture/Composition/PlaneTextureLayer.hpp"

class DebugUI;
class TextureManager;

/** @brief 板ポリテクスチャレイアウトのImGui編集ツール
 * レイヤー配列の保持・矩形のドラッグ編集・プレビュー表示に加え、テンプレート/パターンの
 * JSON保存読込と、PlaneTextureBakerによる合成結果のTextureManagerへの登録までを担当する。
 * Bake結果はTextureManagerの通常のテクスチャとして登録されるため、Mesh側の変更は不要
 * （Mesh::Debug()のTextureコンボが自動的に一覧へ反映する）。
 */
class PlaneTextureLayoutEditor {
public:
    /** @brief 初期化
     * @param _debug DebugUI参照
     * @param _texture テクスチャプレビュー解決用のTextureManager
     * @param _windowName DebugUIメニュー上の識別名（複数インスタンスがある場合は一意にする）
     * @param _aspectRatio 対象板ポリの縦横比（幅/高さ）
     */
    void Initialize(const GESTD::ReferencePtr<DebugUI>& _debug, TextureManager* _texture,
                     const std::string& _windowName, float _aspectRatio);

    /** @brief 対象板ポリの縦横比を変更 */
    void SetAspectRatio(float _aspectRatio);

    /** @brief レイヤーを追加し、追加先インデックスを返す */
    int AddLayer(const PlaneTextureLayer& _layer = {});

    /** @brief レイヤーを削除 */
    void RemoveLayer(int _index);

    /** @brief 現在のレイヤー一覧を取得（合成処理側から参照する用） */
    const std::vector<PlaneTextureLayer>& GetLayers() const { return layers_; }

    /** @brief 毎フレーム呼び出す。DebugUIへウィンドウ内容を登録する */
    void Debug();

private:
    void RenderCanvas();
    void RenderLayerList();
    void RenderLayerInspector();
    void RenderTemplateSection();
    void RenderBakeSection();

    /** @brief textureKeyに対応するImTextureIDを解決する（キー毎に1回だけRegisterTextureする） */
    uint64_t ResolveImTextureId(const std::string& _textureKey);

    /** @brief Assets/Resources 配下の画像ファイルを再スキャンし、選択候補を更新する */
    void RefreshAvailableTextures();

    /** @brief Assets/Data/PlaneTextureLayout/Templates 配下を再スキャンする */
    void RefreshAvailableTemplates();

    /** @brief Assets/Data/PlaneTextureLayout/Patterns/<templateName_> 配下を再スキャンする */
    void RefreshPatterns();

    void SaveTemplate(const std::string& _name);
    void LoadTemplate(const std::string& _name);

    /** @brief 現在のlayers_が持つ変数IDの一覧（重複なし） */
    std::vector<std::string> GetVariableIds() const;

    /** @brief 変数IDに対応する値でlayers_のコピーを上書きする */
    std::vector<PlaneTextureLayer> ResolveLayers(const std::unordered_map<std::string, std::string>& _values) const;

    /** @brief 合成してTextureManagerへ登録する */
    bool BakeAndRegister(const std::vector<PlaneTextureLayer>& _resolvedLayers, const std::string& _outputKey);

    /** @brief 検出済みの全パターンファイルをBakeする */
    void BakeAllPatterns();

    GESTD::ReferencePtr<DebugUI> debug_;
    TextureManager* texture_ = nullptr;
    std::string windowName_;
    float aspectRatio_ = 1.f;

    std::vector<PlaneTextureLayer> layers_;
    int selectedIndex_ = -1;

    std::unordered_map<std::string, uint64_t> textureIdCache_;

    std::vector<std::string> availableTextures_;
    bool availableTexturesScanned_ = false;

    std::string templateName_;
    std::vector<std::string> availableTemplates_;
    bool availableTemplatesScanned_ = false;

    int outputWidth_ = 512;

    std::unordered_map<std::string, std::string> singleBakeValues_;
    std::string singleBakeOutputKey_;

    std::vector<std::string> patternFiles_;
    bool patternsScanned_ = false;
}; // class PlaneTextureLayoutEditor

#endif // PlaneTextureLayoutEditor_HPP_
