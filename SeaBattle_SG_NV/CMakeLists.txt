cmake_minimum_required(VERSION 3.15)
project(SeaBattle)

set(CMAKE_CXX_STANDARD 17)

# ��������� SFML
set(SFML_DIR "C:/SFML/lib/cmake/SFML") # ���� � SFML
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# �������� �����
file(GLOB SOURCES "src/*.cpp")

# ����������� ����
add_executable(SeaBattle ${SOURCES})

# ����������� SFML
target_link_libraries(SeaBattle PRIVATE sfml-graphics sfml-window sfml-system)

# ����������� ��������
add_custom_command(TARGET SeaBattle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:SeaBattle>/assets)