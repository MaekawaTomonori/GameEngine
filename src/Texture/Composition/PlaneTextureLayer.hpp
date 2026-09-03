#ifndef PlaneTextureLayer_HPP_
#define PlaneTextureLayer_HPP_
#include <string>

#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

/** @brief 板ポリテクスチャ合成の1レイヤー
 * rectMin/rectMax は板ポリのUVに対応する0.0-1.0正規化座標。
 */
struct PlaneTextureLayer {
    enum class Type {
        Texture,
        Text,
    };

    Type type = Type::Texture;
    std::string textureKey;
    std::string text;
    /** @brief 空なら固定レイヤー。非空ならパターン適用時にこのIDの値で
     * textureKey（Textureレイヤー）/text（Textレイヤー）が上書きされる
     */
    std::string variableId;
    Vector2 rectMin{ 0.f, 0.f };
    Vector2 rectMax{ 1.f, 1.f };
    Vector4 color{ 1.f, 1.f, 1.f, 1.f };
    int zOrder = 0;
};

#endif // PlaneTextureLayer_HPP_
