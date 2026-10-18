
# =====================================================
# SISTRA - CONFIGURACION DEL BACKEND
# =====================================================

find_package(unofficial-libmariadb CONFIG REQUIRED)

add_executable(SISTRA
        src/main.cpp
        src/database/DatabaseConnection.cpp
)

target_compile_features(SISTRA
        PRIVATE
        cxx_std_20
)

target_include_directories(SISTRA
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(SISTRA
        PRIVATE
        unofficial::libmariadb
)
