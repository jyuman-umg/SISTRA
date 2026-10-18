#include <cstdlib>
#include <iostream>
#include <string>

#include "database/DatabaseConnection.h"

std::string getEnvironmentVariable(const char* name) {
    const char* value = std::getenv(name);

    return value != nullptr ? value : "";
}

int main() {
    std::cout << "====================================="
              << std::endl;
    std::cout << " SISTRA - SISTEMA DE CONTROL DE TRAFICO"
              << std::endl;
    std::cout << "====================================="
              << std::endl;

    const std::string host =
        getEnvironmentVariable("SISTRA_DB_HOST");

    const std::string user =
        getEnvironmentVariable("SISTRA_DB_USER");

    const std::string password =
        getEnvironmentVariable("SISTRA_DB_PASSWORD");

    const std::string database =
        getEnvironmentVariable("SISTRA_DB_NAME");

    const std::string portText =
        getEnvironmentVariable("SISTRA_DB_PORT");

    if (host.empty() || user.empty() ||
        database.empty() || portText.empty()) {
        std::cerr << "Faltan variables de entorno de MySQL."
                  << std::endl;
        return 1;
    }

    unsigned int port;

    try {
        port = static_cast<unsigned int>(
            std::stoul(portText)
        );
    } catch (...) {
        std::cerr << "El puerto de MySQL no es valido."
                  << std::endl;
        return 1;
    }

    DatabaseConnection db;

    if (!db.connect(
            host,
            user,
            password,
            database,
            port
        )) {
        return 1;
    }

    std::cout << std::endl;
    std::cout << "Verificando base de datos..."
              << std::endl;

    if (!db.executeQuery(
            "SELECT DATABASE(), VERSION()"
        )) {
        return 1;
    }

    std::cout << std::endl;
    std::cout << "Calles registradas:"
              << std::endl;

    if (!db.executeQuery(
            "SELECT Nombre FROM Calles ORDER BY Nombre"
        )) {
        return 1;
    }

    db.disconnect();

    std::cout << std::endl;
    std::cout << "Prueba finalizada correctamente."
              << std::endl;

    return 0;
}
