#ifndef SRVHandle_HPP_
#define SRVHandle_HPP_
#include <cstdint>

class SRVManager;

/** @brief SRVスロットの所有権を表すRAIIハンドル
 * 破棄時にSRVManagerへスロットを自動的に返却する
 */
class SRVHandle {
    SRVManager* owner_ = nullptr;
    uint32_t index_ = UINT_MAX;

public:
    SRVHandle() = default;

    /** @brief ハンドルを構築
     * @param _owner 返却先のSRVManager
     * @param _index 確保済みのスロットインデックス
     */
    SRVHandle(SRVManager* _owner, uint32_t _index);

    SRVHandle(const SRVHandle&) = delete;
    SRVHandle& operator=(const SRVHandle&) = delete;

    SRVHandle(SRVHandle&& _other) noexcept;
    SRVHandle& operator=(SRVHandle&& _other) noexcept;

    ~SRVHandle();

    /** @brief 保持しているスロットを明示的に返却する
     */
    void Reset();

    /** @brief 有効なスロットを保持しているか
     * @return 有効な場合true
     */
    bool IsValid() const { return owner_ != nullptr; }

    /** @brief スロットインデックスを取得
     * @return スロットインデックス
     */
    uint32_t Get() const { return index_; }

    operator uint32_t() const { return index_; }
}; // class SRVHandle

#endif // SRVHandle_HPP_
