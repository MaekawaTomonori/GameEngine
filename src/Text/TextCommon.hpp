#ifndef TextCommon_HPP_
#define TextCommon_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include "Math/Vector2.hpp"
#include "src/Common/Common.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/Resource/DX12Resource.hpp"

class Text;
class TextureManager;

/** @brief テキスト描画の内部システムクラス
 *
 * stb_truetype で SDF フォントアトラスを生成し、
 * GPU の StructuredBuffer を使ってグリフをインスタンス描画する。
 * ゲームコードからは直接触らず、Text クラス経由で使用する。
 */
class TextCommon : public Common {
    static constexpr uint32_t kMaxGlyphs      = 2048;
    static constexpr int      kAtlasW         = 512;
    static constexpr int      kAtlasH         = 512;
    static constexpr float    kAtlasFontPx    = 32.f;
    static constexpr int      kSdfPadding     = 4;
    static constexpr uint8_t  kOnEdgeValue    = 180;
    static constexpr float    kPixelDistScale = 16.f;

    struct GlyphInstance {
        float posX, posY;
        float sizeX, sizeY;
        float uvX, uvY;
        float uvW, uvH;
        float r, g, b, a;
    };

    struct ScreenParams {
        float screenW = 1280.f;
        float screenH =  720.f;
        float pad0    =    0.f;
        float pad1    =    0.f;
    };

    struct GlyphMetrics {
        int   atlasX, atlasY;
        int   atlasW, atlasH;
        int   bearingX;   ///< stb xoff: ペン原点からビットマップ左端までのオフセット
        int   bearingY;   ///< stb yoff: ベースラインからビットマップ上端までのオフセット（負 = ベースライン上）
        float advance;    ///< 水平アドバンス（kAtlasFontPx 時のピクセル値）
    };

    TextureManager* texture_ = nullptr;
    SRVManager*     srv_     = nullptr;

    std::vector<uint8_t>                    fontData_;
    void*                                   fontInfo_ = nullptr; ///< stbtt_fontinfo（opaque）
    std::unordered_map<uint32_t, GlyphMetrics> glyphs_;
    float       fontScale_  = 0.f;
    float       fontAscent_ = 0.f;   ///< kAtlasFontPx 時のアセント（ピクセル）
    bool        fontReady_  = false;
    std::string atlasKey_;

    std::unique_ptr<DX12Resource> instanceBuffer_;
    GlyphInstance*                mappedInstances_  = nullptr;
    uint32_t                      glyphSrvIndex_    = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE   glyphGpuHandle_{};

    std::unique_ptr<DX12Resource> screenParamsCb_;
    ScreenParams*                 mappedScreenParams_ = nullptr;

    uint32_t pendingGlyphs_ = 0;

    struct TextData {
        std::string text     = "";
        Vector2     position = {};
        float       fontSize = 32.f;
        Vector4     color    = {1.f, 1.f, 1.f, 1.f};
        bool        visible  = true;
    };

    std::vector<Text*>                        texts_;       ///< 登録済み Text インスタンス一覧（デバッグ表示用）
    std::vector<std::string>                  entryOrder_;  ///< エディター管理エントリの挿入順
    std::unordered_map<std::string, TextData> entries_;     ///< エディター管理エントリ実体

    char fontPathBuf_[256] = "Assets/Fonts/Satoshi-Variable.ttf";

public:
    ~TextCommon() override;

    /** @brief 初期化（Framework から呼ぶ）
     * @param _adapter   DirectXAdapter
     * @param _debugUi   DebugUI（nullptr 可）
     * @param _texture   TextureManager
     * @param _srv       SRVManager
     */
    void Initialize(
        const GESTD::ReferencePtr<DirectXAdapter>& _adapter,
        const GESTD::ReferencePtr<DebugUI>&         _debugUi,
        TextureManager*                             _texture,
        SRVManager*                                 _srv);

    /** @brief フォントファイルを読み込み SDF アトラスを GPU にアップロード
     * @param _path TTF / OTF ファイルパス
     * @return 成功なら true
     */
    bool LoadFont(const std::string& _path);

    /** @brief Text インスタンスを登録（Text コンストラクタから呼ぶ） */
    void RegisterText(Text* _text);

    /** @brief Text インスタンスを解除（Text デストラクタから呼ぶ） */
    void UnregisterText(Text* _text);

    /** @brief テキスト1件をグリフインスタンスとして GPU バッファへ積む
     *
     * Text::Draw() から呼ばれる。scene->Draw() 中に全 Text から積まれた後、
     * Draw(Renderer*) でまとめて描画される。
     */
    void SubmitEntry(
        const std::string& _text,
        float _x, float _y,
        float _fontSize,
        float _r, float _g, float _b, float _a);

    /** @brief 積まれたグリフを 1 回の DrawInstanced でレンダラーに登録
     * @note 呼び出し後 pendingGlyphs_ がリセットされる
     */
    void Draw(Renderer* _renderer) override;

private:
    void Initialize(
        const GESTD::ReferencePtr<DirectXAdapter>& _adapter,
        const GESTD::ReferencePtr<DebugUI>&         _debugUi) override;

    void SetupPipeline();
    void LoadFromJson();
    void SaveToJson();

    static constexpr std::string_view kJsonDir  = "Assets/Data/Text/";
    static constexpr std::string_view kJsonFile = "Assets/Data/Text/texts.json";
};

#endif // TextCommon_HPP_
