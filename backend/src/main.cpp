#include <crow.h>
#include <iostream>
#include "routes/ApiRoutes.h"
#include "routes/VehiculoRoutes.h"
#include "routes/SimulacionRoutes.h"

int main() {
    crow::SimpleApp app;

    std::cout << "====================================="
              << std::endl;
    std::cout << " SISTRA - SISTEMA DE CONTROL DE TRAFICO"
              << std::endl;
    std::cout << "====================================="
              << std::endl;

    registerApiRoutes(app);
    registerVehiculoRoutes(app);
    registerSimulacionRoutes(app);

    std::cout << std::endl;
    std::cout << "Servidor SISTRA iniciado."
              << std::endl;
    std::cout << "Direccion: http://localhost:18080"
              << std::endl;

    app.port(18080).multithreaded().run();

    return 0;
}
