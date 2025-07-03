@echo off
cd ..

if exist vendor\assimp\build rmdir /s /q vendor\assimp\build
mkdir vendor\assimp\build
cd vendor\assimp\build

REM プロジェクト設定に合わせたCMake設定

REM Debug設定でビルド - MTd使用（静的リンク）
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF ^
    -DASSIMP_BUILD_TESTS=OFF ^
    -DCMAKE_CXX_STANDARD=17 ^
    -DCMAKE_CXX_FLAGS_DEBUG="/MTd /EHsc /W4" ^
    -DCMAKE_C_FLAGS_DEBUG="/MTd /EHsc" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DASSIMP_BUILD_ZLIB=ON ^
    -DASSIMP_NO_EXPORT=ON

REM Debugビルド実行
cmake --build . --config Debug

REM Release設定でビルド - MT使用（静的リンク）
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF ^
    -DASSIMP_BUILD_TESTS=OFF ^
    -DCMAKE_CXX_STANDARD=17 ^
    -DCMAKE_CXX_FLAGS_RELEASE="/MT /EHsc /W4" ^
    -DCMAKE_C_FLAGS_RELEASE="/MT /EHsc" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DASSIMP_BUILD_ZLIB=ON ^
    -DASSIMP_NO_EXPORT=ON

REM Releaseビルド実行
cmake --build . --config Release

REM config.hを確認してコピー
if exist include\assimp\config.h (
) else (
    if not exist include\assimp mkdir include\assimp
    echo #ifndef ASSIMP_CONFIG_H > include\assimp\config.h
    echo #define ASSIMP_CONFIG_H >> include\assimp\config.h
    echo #define ASSIMP_BUILD_NO_EXPORT >> include\assimp\config.h
    echo #endif // ASSIMP_CONFIG_H >> include\assimp\config.h
)

pause