#ifndef Heap_HPP_
#define Heap_HPP_
#include <d3d12.h>
#include <inttypes.h>
#include <wrl/client.h>

/** @brief ディスクリプタヒープクラス
 ** DirectX12のディスクリプタヒープを管理
 **/
class Heap {
    ID3D12Device* device_ = nullptr; // Assume this is set elsewhere, or pass it in the constructor
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
    D3D12_DESCRIPTOR_HEAP_TYPE type_ = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Default type

public:
    /** @brief ディスクリプタヒープを作成
     ** @param _device デバイス
     ** @param _type ヒープタイプ
     ** @param _numDescriptors ディスクリプタ数
     ** @param _flags フラグ
     ** @return 成功した場合true
     **/
    bool Create(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, UINT _numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    /** @brief ディスクリプタヒープを取得
     ** @return ディスクリプタヒープポインタ
     **/
    ID3D12DescriptorHeap* Get() const;

    /** @brief CPUディスクリプタハンドルを取得
     ** @param index インデックス
     ** @return CPUディスクリプタハンドル
     **/
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;

    /** @brief GPUディスクリプタハンドルを取得
     ** @param index インデックス
     ** @return GPUディスクリプタハンドル
     **/
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;
}; // class Heap

#endif // Heap_HPP_
