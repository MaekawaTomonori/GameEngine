#ifndef Model_HPP_
#define Model_HPP_
#include <string>

#include "ReferencePtr.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"

class ModelInstance;

/** @brief 3Dモデルクラス（公開ハンドル）
 * 実体（ModelInstance）はEngine側が所有し、本クラスは実体への安全な参照を保持するだけの薄いラッパー。
 * 実体が破棄された後に呼び出しても安全に無視される。
 */
class Model {
    GESTD::ReferencePtr<ModelInstance> instance_;

public:
    Model();
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    Model(Model&& _other) noexcept;
    Model& operator=(Model&& _other) noexcept;

    /** @brief モデルを初期化
     * @param _name モデル名
     */
    void Initialize(const std::string& _name);

    /** @brief モデルの更新処理
     */
    void Update();

    /** @brief マップデータの更新
     */
    void UpdateMapData() const;

    /** @brief モデルを描画
     */
    void Draw() const;

    /** @brief 名前を設定
     * @param _name
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetName(const std::string& _name);

    /** @brief 平行移動を設定
     * @param _translate 平行移動ベクトル
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetTranslate(const Vector3& _translate);

    /** @brief 回転を設定
     * @param _rotate 回転ベクトル
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetRotate(const Vector3& _rotate);

    /** @brief スケールを設定
     * @param _scale スケールベクトル
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetScale(const Vector3& _scale);

    /** @brief 環境マッピング用テクスチャを設定
     * @param _texture テクスチャパス
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetEnvironmentTexture(const std::string& _texture);

    /** @brief テクスチャを設定
     * @param _texture テクスチャパス
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetTexture(const std::string& _texture);

    /** @brief テクスチャのタイリング倍率を設定
     * @param _mul タイリング倍率
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetTilingMultiply(Vector2 _mul);

    /** @brief 色の設定
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetColor(const Vector4& _color);

    /** @brief 所属Canvas名を設定
     * @return メソッドチェーン用の自身への参照
     */
    Model& SetCanvasName(const std::string& _canvasName);

    /** @brief 所属Canvas名を取得 */
    const std::string& GetCanvasName() const;

    /** @brief モデル名の取得
     * @return モデル名
     */
    const std::string& GetName() const;

    /** @brief モデルデータを事前読み込み
     * @param _name モデル名
     */
    static void Load(const std::string& _name);
}; // class Model

#endif // Model_HPP_
