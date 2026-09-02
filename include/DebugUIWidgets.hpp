#ifndef DebugUIWidgets_HPP_
#define DebugUIWidgets_HPP_
#include <string>
#include <vector>

/** @brief ImGui入力ウィジェットのラッパー集
 * デフォルトの `<Widget>[Name]` レイアウトはウィンドウが狭いと名前が読めなくなるため、
 * 名前を上段に、ウィジェットを下段いっぱいに表示するレイアウトへ統一する
 */
namespace DebugUIWidgets {
    bool DragFloat(const char* _label, float* _v, float _speed = 0.01f, float _min = 0.f, float _max = 0.f);
    bool DragFloat3(const char* _label, float* _v, float _speed = 0.01f, float _min = 0.f, float _max = 0.f);
    bool DragInt(const char* _label, int* _v, float _speed, int _min, int _max);
    bool ColorEdit4(const char* _label, float* _v);
    bool Checkbox(const char* _label, bool* _v);
    bool Combo(const char* _label, int* _current, const char* const _items[], int _count);

    /** @brief キー一覧からコンボボックスで選択させる（先頭は未設定扱い）
     * @param _label 表示名
     * @param _keys 選択可能なキー一覧（空なら選べない旨を表示する）
     * @param _selected 選択中のキー。変更されると書き換えられる
     * @return 選択が変更されたか
     */
    bool KeyCombo(const char* _label, const std::vector<std::string>& _keys, std::string& _selected);
} // namespace DebugUIWidgets

#endif // DebugUIWidgets_HPP_
