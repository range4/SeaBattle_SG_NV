cmake_minimum_required(VERSION 3.10)
project(SeaBattle)

set(CMAKE_CXX_STANDARD 17)

# Настройка SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Board.cpp
    src/Ship.cpp
    src/AI.cpp
)

# Исполняемый файл
add_executable(SeaBattle ${SOURCES})

# Подключение SFML
target_link_libraries(SeaBattle sfml-graphics sfml-window sfml-system)

# Копирование ресурсов после сборки
add_custom_command(TARGET SeaBattle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:SeaBattle>/assets)