#ifndef Heap_HPP_
#define Heap_HPP_
#include <d3d12.h>
#include <inttypes.h>
#include <wrl/client.h>

/// <summary>
/// ディスクリプタヒープクラス
/// DirectX12のディスクリプタヒープを管理
/// </summary>
class Heap {
    ID3D12Device* device_ = nullptr; // Assume this is set elsewhere, or pass it in the constructor
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
    D3D12_DESCRIPTOR_HEAP_TYPE type_ = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Default type

public:
    /// <summary>
    /// ディスクリプタヒープを作成
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_type">ヒープタイプ</param>
    /// <param name="_numDescriptors">ディスクリプタ数</param>
    /// <param name="_flags">フラグ</param>
    /// <returns>成功した場合true</returns>
    bool Create(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, UINT _numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    /// <summary>
    /// ディスクリプタヒープを取得
    /// </summary>
    /// <returns>ディスクリプタヒープポインタ</returns>
    ID3D12DescriptorHeap* Get() const;

    /// <summary>
    /// CPUディスクリプタハンドルを取得
    /// </summary>
    /// <param name="index">インデックス</param>
    /// <returns>CPUディスクリプタハンドル</returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;

    /// <summary>
    /// GPUディスクリプタハンドルを取得
    /// </summary>
    /// <param name="index">インデックス</param>
    /// <returns>GPUディスクリプタハンドル</returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;
}; // class Heap

#endif // Heap_HPP_
