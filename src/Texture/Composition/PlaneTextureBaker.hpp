#ifndef PlaneTextureBaker_HPP_
#define PlaneTextureBaker_HPP_
#include <cstdint>
#include <vector>

#include "src/Texture/Composition/PlaneTextureLayer.hpp"

/** @brief PlaneTextureLayer の配列を1枚のRGBA8ピクセルバッファへ合成する。
 * ImGui・TextureManagerには依存しない。
 */
namespace PlaneTextureBaker {
    /** @brief レイヤーをzOrder昇順で合成する
     * @param _layers 合成するレイヤー
     * @param _width 出力幅
     * @param _height 出力高さ
     * @param _outPixels 成功時 _width*_height*4 (RGBA8) で埋められる
     * @return バッファ確保等の致命的な失敗以外は true（個々のレイヤーの読み込み失敗はスキップする）
     */
    bool Bake(const std::vector<PlaneTextureLayer>& _layers, uint32_t _width, uint32_t _height,
              std::vector<uint8_t>& _outPixels);
} // namespace PlaneTextureBaker

#endif // PlaneTextureBaker_HPP_
