cmake_minimum_required(VERSION 3.21)

project(SISTRA
        VERSION 1.0.0
        DESCRIPTION "Sistema Inteligente de Simulacion y Control de Trafico Vehicular"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(backend)
