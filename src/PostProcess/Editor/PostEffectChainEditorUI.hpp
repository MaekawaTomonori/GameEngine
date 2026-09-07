#ifndef PostEffectChainEditorUI_HPP_
#define PostEffectChainEditorUI_HPP_
#include <string>

#include "ReferencePtr.hpp"

class PostEffectChain;
class PostEffectFactory;

/** @brief PostEffectChainの常時構成エフェクトを編集するUIパネル
 * Canvasの詳細編集とOverlay編集の両方から共通で使う（Add / 一覧 / 有効切替 / 並び替え / 削除 / パラメータ編集 / Save・Load Config）
 * @param _chain 編集対象のチェーン
 * @param _factory エフェクト生成用ファクトリ
 * @param _selectedEffect 選択中エフェクトタイプ名（呼び出し側で保持する状態）
 * @param _selectedNewEffectType Add Effectコンボの選択インデックス（呼び出し側で保持する状態）
 */
void RenderPostEffectChainEditor(PostEffectChain* _chain, const GESTD::ReferencePtr<PostEffectFactory>& _factory, std::string& _selectedEffect, int& _selectedNewEffectType);

#endif // PostEffectChainEditorUI_HPP_
