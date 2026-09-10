#ifndef Heap_HPP_
#define Heap_HPP_
#include <d3d12.h>
#include <inttypes.h>
#include <vector>
#include <wrl/client.h>

/** @brief ディスクリプタヒープクラス
 * DirectX12のディスクリプタヒープを管理
 */
class Heap {
    ID3D12Device* device_ = nullptr; // Assume this is set elsewhere, or pass it in the constructor
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
    D3D12_DESCRIPTOR_HEAP_TYPE type_ = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Default type

    uint32_t numDescriptors_ = 0;
    uint32_t nextIndex_ = 0;
    std::vector<uint32_t> freeIndices_;

public:
    /** @brief ディスクリプタヒープを作成
     * @param _device デバイス
     * @param _type ヒープタイプ
     * @param _numDescriptors ディスクリプタ数
     * @param _flags フラグ
     * @return 成功した場合true
     */
    bool Create(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, UINT _numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    /** @brief 空いているスロットを確保する
     * 解放済みスロットがあればそれを再利用し、無ければ未使用の新しいスロットを返す
     * @return 確保したスロットのインデックス
     */
    uint32_t Allocate();

    /** @brief スロットを解放し、以後のAllocate()で再利用可能にする
     * @param _index 解放するスロットのインデックス
     */
    void Free(uint32_t _index);

    /** @brief 空きスロットが無いかどうか
     * @return 空きが無ければtrue
     */
    bool IsFull() const;

    /** @brief ディスクリプタヒープを取得
     * @return ディスクリプタヒープポインタ
     */
    ID3D12DescriptorHeap* Get() const;

    /** @brief CPUディスクリプタハンドルを取得
     * @param index インデックス
     * @return CPUディスクリプタハンドル
     */
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t _index) const;

    /** @brief GPUディスクリプタハンドルを取得
     * @param index インデックス
     * @return GPUディスクリプタハンドル
     */
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t _index) const;
}; // class Heap

#endif // Heap_HPP_
