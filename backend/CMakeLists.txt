# =====================================================
# SISTRA - CONFIGURACION DEL BACKEND
# =====================================================

find_package(unofficial-libmariadb CONFIG REQUIRED)
find_package(Crow CONFIG REQUIRED)

add_executable(SISTRA
        src/main.cpp
        src/database/DatabaseConnection.cpp
        src/routes/ApiRoutes.cpp
        src/routes/VehiculoRoutes.cpp
        src/routes/SimulacionRoutes.cpp
        src/simulation/SimulationEngine.cpp
        src/traffic/TrafficLightController.cpp
        src/traffic/SpeedControlService.cpp
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
        Crow::Crow
)

# =====================================================
# BIBLIOTECAS DE WINDOWS
# =====================================================

if(WIN32)
    target_link_libraries(SISTRA
            PRIVATE
            ws2_32
            mswsock
    )
endif()