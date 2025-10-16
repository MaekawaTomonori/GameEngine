#ifndef DX12Resource_HPP_
#define DX12Resource_HPP_
#include <wrl/client.h>
#include <d3d12.h>

/// <summary>
/// DirectX12リソースラッパークラス
/// リソース状態遷移を管理
/// </summary>
class DX12Resource {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    D3D12_RESOURCE_STATES state_ = D3D12_RESOURCE_STATE_COMMON;
public:
    /// <summary>
    /// リソースを作成
    /// </summary>
    /// <param name="_resource">リソースポインタ</param>
    /// <param name="_state">初期リソース状態</param>
    void Create(ID3D12Resource* _resource, D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_COMMON);

    /// <summary>
    /// リソースを作成
    /// </summary>
    /// <param name="_resource">リソースのComPtr</param>
    /// <param name="_state">初期リソース状態</param>
    void Create(const Microsoft::WRL::ComPtr<ID3D12Resource>& _resource, D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_COMMON);

    /// <summary>
    /// リソース状態を変更
    /// </summary>
    /// <param name="_command">コマンドリスト</param>
    /// <param name="_state">新しいリソース状態</param>
    void ChangeState(ID3D12GraphicsCommandList* _command, D3D12_RESOURCE_STATES _state);

    /// <summary>
    /// リソースを取得
    /// </summary>
    /// <returns>リソースポインタ</returns>
    ID3D12Resource* Get() const;
}; // class DX12Resource

#endif // DX12Resource_HPP_
