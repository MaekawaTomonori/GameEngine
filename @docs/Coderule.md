## ファイル拡張子規則

### C++ファイル
- ヘッダーファイル: `.hpp`
- 実装ファイル: `.cpp`
- `.h`や`.cc`は使用しない

### シェーダーファイル
- 共通インクルード: `.hlsli`
- 頂点シェーダー: `.VS.hlsl`
- ピクセルシェーダー: `.PS.hlsl`
- コンピュートシェーダー: `.CS.hlsl`

## ヘッダーガード規則
- `#pragma once` ではなく従来型ガードを使用
- 形式: `#ifndef [FILE_NAME]_HPP_` / `#define [FILE_NAME]_HPP_`
- ファイル名は全て大文字、パスは含めない

```c++
#ifndef SAMPLE_HPP_
#define SAMPLE_HPP_

// ...

#endif // SAMPLE_HPP_
```

## 命名規則

### 基本方針
- **クラス/構造体/列挙型**: UpperCamelCase（例: `DirectXAdapter`, `ModelRepository`）
- **関数**: UpperCamelCase()（例: `Initialize()`, `GetDevice()`）
- **定数**: ALL_UPPER（例: `MAX_TEXTURE_COUNT`, `PI`）
- **引数**: _lowerCamelCase（例: `_position`, `_fileName`）
- **メンバ変数**: lowerCamelCase_（例: `position_`, `velocity_`）

### 具体例
```c++
class TextureManager {
    // メンバ変数: lowerCamelCase_
    int32_t maxTextureCount_;
    std::string basePath_;

public:
    // 定数: ALL_UPPER
    static constexpr int32_t MAX_TEXTURE_COUNT = 256;

    // 関数: UpperCamelCase()
    // 引数: _lowerCamelCase
    void LoadTexture(const std::string& _filePath);
    int32_t GetTextureCount() const;
};
```

## クラス構造規則

### アクセス指定子の配置
- `public`/`private`/`protected`はクラスと同じ階層に記述
- それ以外のメンバはインデントを上げる

### ヘッダーファイルの記述順序
1. private メンバ変数
2. public コンストラクタ/デストラクタ
3. public 関数
4. public アクセサ（Get/Set関数）
5. private 関数

```c++
class Sample {
    // Member Variable
    int32_t id_;

public:
    // Functions
    void Register();
    // Accessor
    int32_t GetId() const;

private:
    // private funcs
    void CreateID();
};
```

## コードフォーマット規則

### 制御構文
- `if`, `for`, `while`, `switch`の後はスペースを入れる
- `{`は改行せず同じ行に記述
- `}`の後の`else`は改行せず同じ行に配置

```c++
if (condition) {
    // Do something...
} else if (otherCondition) {
    // Do something else...
} else {
    // Default case...
}

for (int i = 0; i < count; i++) {
    // Loop body...
}
```

### ポインタ・参照の記述
- 型に付ける形式を使用（型* 変数名、型& 変数名）
- 変数名に付ける形式は使用しない

```c++
// ✅ 正しい
int* ptr;
int& ref;

// ❌ 誤り
int *ptr;
int &ref;
```

### Lambda式
- 基本的にキャプチャは`[&]`または`[this]`を使用
- 関数引数として渡す場合も同様

```c++
// 基本形
[&]() {
    // Do Something...
}

// 関数の引数としてlambdaを渡す場合
example([this]() {
    // Do something...
});

// 明示的なキャプチャが必要な場合
[&position, scale]() {
    // Use position and scale...
}
```

## 関数設計規則

### 引数の記述
- 接頭辞として`_`を付ける（例: `_position`, `_fileName`）
- 引数が多い場合でも改行せず同じ行に書ききる
- const参照を積極的に使用（コピーコストを避ける）

```c++
// ✅ 正しい
void SetTransform(const Vector3& _position, const Vector3& _rotation, const Vector3& _scale);

// ❌ 誤り（改行している）
void SetTransform(
    const Vector3& _position,
    const Vector3& _rotation,
    const Vector3& _scale
);
```

### メソッドチェーン
- 関数の返り値で自身の参照を返し、連続した設定を可能にする
- 流暢なインターフェース（Fluent Interface）パターン

```c++
class Transform {
public:
    Transform& SetPosition(const Vector3& _pos) {
        position_ = _pos;
        return *this;
    }

    Transform& SetScale(const Vector3& _scale) {
        scale_ = _scale;
        return *this;
    }
};

// 使用例
Transform transform;
transform.SetPosition({10, 200, 0})
    .SetScale({1.5f, 1.5f, 1.5f});
```

## コメント規則

### Doxygenスタイル
- `/**` スタイルを使用（`///`は使用しない）
- `@brief`形式を使用（`<Summary>`形式は非推奨）
- 関数の説明には`@param`、`@return`を使用

```c++
// ❌ 非推奨（古い形式）
/// <Summary>
/// テクスチャを読み込む
/// </Summary>
/// <param name="filePath">ファイルパス</param>
/// <returns>成功時true</returns>

// ✅ 推奨（新しい形式）
/** @brief テクスチャを読み込む
 * @param _filePath テクスチャファイルのパス
 * @return 読み込み成功時true
 */
bool LoadTexture(const std::string& _filePath);
```

### インラインコメント
- 複雑なロジックには適切なコメントを付ける
- 自明なコードにはコメント不要

```c++
// GPU同期待ち（フレーム処理完了を保証）
commandQueue->Signal(fence.Get(), fenceValue);
fence->SetEventOnCompletion(fenceValue, fenceEvent);
WaitForSingleObject(fenceEvent, INFINITE);
```

## includeディレクティブ規則

### include順序
1. 同一クラスのヘッダー（例: `#include "Sample.hpp"`）
2. 標準ライブラリ（例: `#include <vector>`, `#include <memory>`）
3. 外部ライブラリ（例: `#include <d3d12.h>`, `#include <DirectXMath.h>`）
4. プロジェクト内の他クラス

### include方針
- **必要最低限のみをinclude**
- STLは他のインクルード先で使われていても**明示的にinclude**
- 前方宣言を活用してincludeを削減
- ヘッダーファイルでは実装の詳細を隠蔽

```c++
// Sample.hpp
#ifndef SAMPLE_HPP_
#define SAMPLE_HPP_

// 1. 標準ライブラリ
#include <memory>
#include <string>
#include <vector>

// 2. 外部ライブラリ
#include <d3d12.h>

// 3. プロジェクト内
#include "Vector3.hpp"

// 前方宣言（includeを避ける）
class TextureManager;

class Sample {
    // ...
};

#endif // SAMPLE_HPP_
```

## 名前空間規則

### ネストした名前空間
- ネストは名前空間ごとにインデントを上げる

```c++
// ネストごとにインデント
namespace Engine {
    namespace Graphics {
        void Render();
    }
    namespace Physics {
        void Update();
    }
}
```

## エラー処理規則

### Assert使用方針
- 標準`assert`ではなく`Utils::Alert`を使用
- リリースビルドでも重要なチェックは残す
- GPU関連エラーは明示的なログ出力を伴う

```c++
// ❌ 非推奨
assert(device != nullptr);

// ✅ 推奨
Utils::Alert(device != nullptr, "DirectX device creation failed");

// ✅ より詳細なエラー情報
if (FAILED(hr)) {
    Utils::Alert(false, "CreateCommittedResource failed: HRESULT = 0x%08X", hr);
}
```

### エラーハンドリングの方針
- 例外は使用しない（パフォーマンス優先）
- エラーは戻り値またはbool値で返す
- 致命的エラーは`Utils::Alert`でログ出力後に終了
