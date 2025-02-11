cmake_minimum_required(VERSION 3.10)
project(GameOfLife3D)

# Указываем стандарт C++
set(CMAKE_CXX_STANDARD 17)

# Устанавливаем поддержку Unicode
add_definitions(-DUNICODE -D_UNICODE)

# Устанавливаем типы конфигураций
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Устанавливаем путь для установки
# set(CMAKE_INSTALL_PREFIX "C:/Program Files/GameOfLife3D" CACHE PATH "Installation Prefix")

# Находим все файлы исходников по категориям
file(GLOB GAME_SOURCES "game/*.cpp" "game/*.h")
file(GLOB RENDERING_SOURCES "rendering/*.cpp" "rendering/*.h")
file(GLOB MATH_SOURCES "mathematics/*.cpp" "mathematics/*.h")
file(GLOB WINDOWING_SOURCES "windowing/*.cpp" "windowing/*.h")
file(GLOB SYSTEM_SOURCES "system/*.cpp" "system/*.h")
file(GLOB RES_SOURCE_DIR "res/*")

# Убедитесь, что не включены файлы из examples
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*examples.*")
list(FILTER RENDERING_SOURCES EXCLUDE REGEX ".*examples.*")
list(FILTER MATH_SOURCES EXCLUDE REGEX ".*examples.*")
list(FILTER WINDOWING_SOURCES EXCLUDE REGEX ".*examples.*")
list(FILTER SYSTEM_SOURCES EXCLUDE REGEX ".*examples.*")

# Добавляем файлы ресурсов
set(RESOURCE_FILES 
    "res/resource.h" 
    "res/life.rc" 
)

# Явно указываем необходимые файлы ImGui
set(IMGUI_SOURCES 
    "${PROJECT_SOURCE_DIR}/external/imgui/imgui.cpp" 
    "${PROJECT_SOURCE_DIR}/external/imgui/imgui.h" 
    "${PROJECT_SOURCE_DIR}/external/imgui/imgui_draw.cpp" 
    "${PROJECT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp" 
    "${PROJECT_SOURCE_DIR}/external/imgui/imgui_tables.cpp" 
    "${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_win32.cpp" 
    "${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_win32.h" 
    "${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp" 
    "${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.h"
)

# Добавляем файлы из папки res
file(GLOB RES_FILES "res/*")

# Указываем директорию для выходных файлов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Создаем исполняемый файл
add_executable(${PROJECT_NAME} 
    ${GAME_SOURCES} 
    ${RENDERING_SOURCES} 
    ${MATH_SOURCES} 
    ${WINDOWING_SOURCES} 
    ${SYSTEM_SOURCES} 
    ${RESOURCE_FILES} 
    ${IMGUI_SOURCES} 
    ${RES_FILES}
    "main.cpp"
)

set(CMAKE_WINMAIN "wWinMain")  # Указываем точку входа

# Устанавливаем, что это приложение Windows
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Добавляем директории с заголовочными файлами
include_directories(
    ${PROJECT_SOURCE_DIR}/game
    ${PROJECT_SOURCE_DIR}/rendering
    ${PROJECT_SOURCE_DIR}/mathematics
    ${PROJECT_SOURCE_DIR}/windowing
    ${PROJECT_SOURCE_DIR}/system
    ${PROJECT_SOURCE_DIR}/external/imgui
    ${PROJECT_SOURCE_DIR}/external/imgui/backends
    ${PROJECT_SOURCE_DIR}/res
)

# Если используете Windows, компиляция ресурсов
if(WIN32)
    # Компилируем RC файл
    target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/res/life.rc)
endif()

# Группировка файлов в VS по папкам
source_group("Source Files\\game" FILES ${GAME_SOURCES})
source_group("Source Files\\rendering" FILES ${RENDERING_SOURCES})
source_group("Source Files\\mathematics" FILES ${MATH_SOURCES})
source_group("Source Files\\windowing" FILES ${WINDOWING_SOURCES})
source_group("Source Files\\system" FILES ${SYSTEM_SOURCES})
source_group("Source Files\\res" FILES ${RES_SOURCE_DIR})

# Группировка только для нужных файлов ImGui
source_group("external\\imgui" FILES 
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui.h
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_tables.cpp
)
source_group("external\\imgui\\backends" FILES 
    ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_win32.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_win32.h
    ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.h
)