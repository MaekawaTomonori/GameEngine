cmake_minimum_required(VERSION 3.10)
project(imgui)

set(SOURCE_FILES
    imconfig.h
    imgui.h
    imgui_impl_dx12.h
    imgui_impl_win32.h
    imgui_internal.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_impl_dx12.cpp
    imgui_impl_win32.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
