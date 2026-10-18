
#include <crow.h>
#include <crow/middlewares/cors.h>

#include <iostream>

#include "routes/ApiRoutes.h"
#include "routes/VehiculoRoutes.h"
#include "routes/SimulacionRoutes.h"

int main() {

    // =====================================================
    // CONFIGURACION DEL SERVIDOR
    // =====================================================

    crow::App<crow::CORSHandler> app;

    // =====================================================
    // CONFIGURACION CORS
    // =====================================================

    auto& cors = app.get_middleware<crow::CORSHandler>();

    cors.global()
        .origin("http://localhost:63342")
        .methods(
            crow::HTTPMethod::GET,
            crow::HTTPMethod::POST,
            crow::HTTPMethod::OPTIONS
        )
        .headers("Content-Type");

    // =====================================================
    // ENCABEZADO DEL SISTEMA
    // =====================================================

    std::cout << "====================================="
              << std::endl;

    std::cout << " SISTRA - SISTEMA DE CONTROL DE TRAFICO"
              << std::endl;

    std::cout << "====================================="
              << std::endl;

    // =====================================================
    // REGISTRO DE RUTAS
    // =====================================================

    registerApiRoutes(app);
    registerVehiculoRoutes(app);
    registerSimulacionRoutes(app);

    // =====================================================
    // INICIAR SERVIDOR
    // =====================================================

    std::cout << std::endl;

    std::cout << "Servidor SISTRA iniciado."
              << std::endl;

    std::cout << "Direccion: http://localhost:18080"
              << std::endl;

    app.port(18080).multithreaded().run();

    return 0;
}
