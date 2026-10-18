#ifndef TEXTUREHANDLE_HPP_
#define TEXTUREHANDLE_HPP_
#include <cstdint>

/** @brief テクスチャの読み込み状態
 */
enum class TextureState{
    NotReady,
    Ready,
    Failed,
};

/** @brief テクスチャへの軽量ハンドル（コピー可能・非所有）
 * 文字列の解決は要求時のみ。以降の参照はロックも文字列操作も伴わない。
 */
class TextureHandle{
    static constexpr uint32_t INVALID_ID = UINT32_MAX;

    uint32_t id_ = INVALID_ID;

public:
    TextureHandle() = default;
    explicit TextureHandle(const uint32_t _id) : id_(_id){
    }

    /** @brief SRVインデックスを取得
     * @return Readyなら実体、それ以外は既定テクスチャのインデックス
     */
    uint32_t GetSrvIndex() const;

    TextureState GetState() const;
    bool IsReady() const;

    bool IsValid() const{
        return id_ != INVALID_ID;
    }

    uint32_t GetId() const{
        return id_;
    }
}; // class TextureHandle

#endif // TEXTUREHANDLE_HPP_
