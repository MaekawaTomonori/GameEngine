#ifndef DX12Resource_HPP_
#define DX12Resource_HPP_
#include <wrl/client.h>
#include <d3d12.h>

/** @brief DirectX12リソースラッパークラス
 ** リソース状態遷移を管理
 **/
class DX12Resource {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    D3D12_RESOURCE_STATES state_ = D3D12_RESOURCE_STATE_COMMON;
public:
    /** @brief リソースを作成
     ** @param _resource リソースポインタ
     ** @param _state 初期リソース状態
     **/
    void Create(ID3D12Resource* _resource, D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_COMMON);

    /** @brief リソースを作成
     ** @param _resource リソースのComPtr
     ** @param _state 初期リソース状態
     **/
    void Create(const Microsoft::WRL::ComPtr<ID3D12Resource>& _resource, D3D12_RESOURCE_STATES _state = D3D12_RESOURCE_STATE_COMMON);

    /** @brief リソース状態を変更
     ** @param _command コマンドリスト
     ** @param _state 新しいリソース状態
     **/
    void ChangeState(ID3D12GraphicsCommandList* _command, D3D12_RESOURCE_STATES _state);

    /** @brief リソースを取得
     ** @return リソースポインタ
     **/
    ID3D12Resource* Get() const;
}; // class DX12Resource

#endif // DX12Resource_HPP_
