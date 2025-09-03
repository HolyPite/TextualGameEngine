cmake_minimum_required(VERSION 3.15)
project(jeu_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(jeu
  src/main.cpp
  src/Histoire.cpp
  src/utils.cpp
  src/Competence.cpp
  src/Entite.cpp
  src/ui.cpp
  src/Objet.cpp
)

target_include_directories(jeu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
