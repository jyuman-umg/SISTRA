#include "routes/ApiRoutes.h"
#include "database/DatabaseConnection.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

    std::string getEnvironmentVariable(const char* name) {
        const char* value = std::getenv(name);

        return value != nullptr ? value : "";
    }

}

void registerApiRoutes(crow::SimpleApp& app) {

    // =====================================================
    // ENDPOINT: ESTADO DEL SERVIDOR
    // =====================================================

    CROW_ROUTE(app, "/api/health")
    ([]() {
        crow::json::wvalue response;

        response["success"] = true;
        response["message"] =
            "Servidor SISTRA funcionando correctamente";
        response["system"] = "SISTRA";
        response["version"] = "1.0.0";

        return response;
    });

    // =====================================================
    // ENDPOINT: INFORMACION GENERAL DEL SISTEMA
    // =====================================================

    CROW_ROUTE(app, "/api/sistema")
    ([]() {
        crow::json::wvalue response;

        response["nombre"] = "SISTRA";
        response["descripcion"] =
            "Sistema Inteligente de Simulacion y Control de Trafico Vehicular";

        response["proyecto"] = 18;
        response["variante"] = "A";
        response["tipoInterseccion"] = "Semaforizada";
        response["cantidadCalles"] = 4;

        return response;
    });

    // =====================================================
    // ENDPOINT: CONSULTAR CALLES DESDE MYSQL
    // =====================================================

    CROW_ROUTE(app, "/api/calles")
    ([]() {
        crow::json::wvalue response;

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

            response["success"] = false;
            response["message"] =
                "Faltan variables de entorno de MySQL";

            return crow::response(500, response);
        }

        unsigned int port;

        try {
            unsigned long parsedPort = std::stoul(portText);

            if (parsedPort == 0 || parsedPort > 65535) {
                throw std::out_of_range("Puerto invalido");
            }

            port = static_cast<unsigned int>(parsedPort);
        } catch (...) {
            response["success"] = false;
            response["message"] = "Puerto de MySQL invalido";

            return crow::response(500, response);
        }

        DatabaseConnection db;

        if (!db.connect(
                host,
                user,
                password,
                database,
                port
            )) {

            response["success"] = false;
            response["message"] =
                "No se pudo establecer conexion con MySQL";

            return crow::response(500, response);
        }

        try {
            const auto calles = db.select(
                "SELECT IdCalle, Nombre, DireccionEntrada, "
                "DireccionSalida, EstadoRegistro "
                "FROM Calles "
                "WHERE EstadoRegistro = 'A' "
                "ORDER BY IdCalle"
            );

            crow::json::wvalue::list listaCalles;

            for (const auto& calle : calles) {
                crow::json::wvalue item;

                item["idCalle"] =
                    std::stoi(calle.at("IdCalle"));

                item["nombre"] =
                    calle.at("Nombre");

                item["direccionEntrada"] =
                    calle.at("DireccionEntrada");

                item["direccionSalida"] =
                    calle.at("DireccionSalida");

                item["estadoRegistro"] =
                    calle.at("EstadoRegistro");

                listaCalles.push_back(std::move(item));
            }

            response["success"] = true;
            response["message"] =
                "Calles consultadas correctamente";
            response["total"] =
                static_cast<int>(calles.size());
            response["data"] =
                std::move(listaCalles);

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr << "Error consultando calles: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["message"] =
                "Ocurrio un error al consultar las calles";

            return crow::response(500, response);
        }
    });


    // =====================================================
    // ENDPOINT: ESTADO DE LA BASE DE DATOS
    // =====================================================

    CROW_ROUTE(app, "/api/database/status")
    ([]() {
        crow::json::wvalue response;

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

            response["success"] = false;
            response["connected"] = false;
            response["message"] =
                "Faltan variables de entorno de MySQL";

            return crow::response(503, response);
        }

        unsigned int port;

        try {
            const unsigned long parsedPort =
                std::stoul(portText);

            if (parsedPort == 0 || parsedPort > 65535) {
                throw std::out_of_range("Puerto invalido");
            }

            port = static_cast<unsigned int>(parsedPort);

        } catch (...) {
            response["success"] = false;
            response["connected"] = false;
            response["message"] =
                "Configuracion de puerto invalida";

            return crow::response(503, response);
        }

        DatabaseConnection db;

        if (!db.connect(
                host,
                user,
                password,
                database,
                port
            )) {

            response["success"] = false;
            response["connected"] = false;
            response["message"] =
                "No se pudo conectar con MySQL";

            return crow::response(503, response);
        }

        try {
            db.select("SELECT 1 AS Estado");

            response["success"] = true;
            response["connected"] = true;
            response["database"] = database;
            response["message"] =
                "Conexion con MySQL funcionando correctamente";

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr << "Error verificando MySQL: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["connected"] = false;
            response["message"] =
                "Error al verificar la base de datos";

            return crow::response(503, response);
        }
    });


}
