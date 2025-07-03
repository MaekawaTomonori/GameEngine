cd ..

if not exist vendor\assimp\build mkdir vendor\assimp\build
cd vendor\assimp\build

REM Debug設定でビルド - MultiThreadedDebugを使用
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF ^
    -DASSIMP_BUILD_TESTS=OFF ^
    -DCMAKE_CXX_FLAGS="/MTd /EHsc /std:c++20 /Zc:__cplusplus /W4 /WX" ^
    -DCMAKE_C_FLAGS="/MTd /EHsc" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DASSIMP_BUILD_ZLIB=ON ^
    -DASSIMP_NO_EXPORT=ON ^
    -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF ^
    -DASSIMP_BUILD_OBJ_IMPORTER=ON ^
    -DASSIMP_BUILD_GLTF_IMPORTER=ON

REM Debugビルド実行
echo Debugビルド実行中...
cmake --build . --config Debug

REM Release設定でビルド - MultiThreadedを使用
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF ^
    -DASSIMP_BUILD_TESTS=OFF ^
    -DCMAKE_CXX_FLAGS="/MT /EHsc /std:c++20 /Zc:__cplusplus /W4 /WX" ^
    -DCMAKE_C_FLAGS="/MT /EHsc" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DASSIMP_BUILD_ZLIB=ON ^
    -DASSIMP_NO_EXPORT=ON ^
    -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF ^
    -DASSIMP_BUILD_OBJ_IMPORTER=ON ^
    -DASSIMP_BUILD_GLTF_IMPORTER=ON

REM Releaseビルド実行
echo Releaseビルド実行中...
cmake --build . --config Release