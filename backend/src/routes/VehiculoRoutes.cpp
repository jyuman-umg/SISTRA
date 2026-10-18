#include "routes/VehiculoRoutes.h"
#include "database/DatabaseConnection.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <algorithm>
#include <cctype>
#include <regex>
#include <cstdint>

namespace {

    std::string getEnvironmentVariable(const char* name) {
        const char* value = std::getenv(name);
        return value != nullptr ? value : "";
    }

    bool connectDatabase(DatabaseConnection& db) {
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
            return false;
        }

        try {
            const unsigned long parsedPort = std::stoul(portText);

            if (parsedPort == 0 || parsedPort > 65535) {
                return false;
            }

            return db.connect(
                host,
                user,
                password,
                database,
                static_cast<unsigned int>(parsedPort)
            );

        } catch (const std::exception&) {
            return false;
        }
    }

    crow::json::wvalue mapVehiculo(
        const std::map<std::string, std::string>& vehiculo
    ) {
        crow::json::wvalue item;

        item["idVehiculo"] =
            std::stoi(vehiculo.at("IdVehiculo"));

        item["placa"] =
            vehiculo.at("Placa");

        item["tipoVehiculo"] =
            vehiculo.at("TipoVehiculo");

        item["fechaRegistro"] =
            vehiculo.at("FechaRegistro");

        item["estadoRegistro"] =
            vehiculo.at("EstadoRegistro");

        return item;
    }


    std::string trim(const std::string& value) {
        const std::size_t start =
            value.find_first_not_of(" \t\n\r");

        if (start == std::string::npos) {
            return "";
        }

        const std::size_t end =
            value.find_last_not_of(" \t\n\r");

        return value.substr(start, end - start + 1);
    }

    std::string normalizePlaca(std::string placa) {
        placa = trim(placa);

        std::transform(
            placa.begin(),
            placa.end(),
            placa.begin(),
            [](unsigned char character) {
                return static_cast<char>(
                    std::toupper(character)
                );
            }
        );

        return placa;
    }

    bool isValidPlaca(const std::string& placa) {
        // Formato inicial de SISTRA:
        // entre 5 y 10 caracteres alfanumericos.
        static const std::regex pattern(
            "^[A-Z0-9]{5,10}$"
        );

        return std::regex_match(placa, pattern);
    }

    bool isValidTipoVehiculo(const std::string& tipo) {
        return tipo == "Automovil" ||
               tipo == "Motocicleta" ||
               tipo == "Camion" ||
               tipo == "Bus";
    }


}

void registerVehiculoRoutes(crow::App<crow::CORSHandler>& app) {

    // =====================================================
    // ENDPOINT: CONSULTAR TODOS LOS VEHICULOS
    // =====================================================

    CROW_ROUTE(app, "/api/vehiculos")
    .methods(crow::HTTPMethod::GET)
    ([]() {
        crow::json::wvalue response;

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            response["success"] = false;
            response["message"] =
                "No se pudo conectar con MySQL";

            return crow::response(503, response);
        }

        try {
            const auto vehiculos = db.select(
                "SELECT IdVehiculo, Placa, TipoVehiculo, "
                "FechaRegistro, EstadoRegistro "
                "FROM Vehiculos "
                "WHERE EstadoRegistro = 'A' "
                "ORDER BY IdVehiculo"
            );

            crow::json::wvalue::list listaVehiculos;

            for (const auto& vehiculo : vehiculos) {
                listaVehiculos.push_back(
                    mapVehiculo(vehiculo)
                );
            }

            response["success"] = true;
            response["message"] =
                "Vehiculos consultados correctamente";

            response["total"] =
                static_cast<int>(vehiculos.size());

            response["data"] =
                std::move(listaVehiculos);

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr << "Error consultando vehiculos: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["message"] =
                "Error al consultar vehiculos";

            return crow::response(500, response);
        }
    });

    // =====================================================
    // ENDPOINT: CONSULTAR VEHICULO POR ID
    // =====================================================

    CROW_ROUTE(app, "/api/vehiculos/<int>")
    .methods(crow::HTTPMethod::GET)
    ([](int idVehiculo) {
        crow::json::wvalue response;

        if (idVehiculo <= 0) {
            response["success"] = false;
            response["message"] =
                "El identificador debe ser mayor que cero";

            return crow::response(400, response);
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            response["success"] = false;
            response["message"] =
                "No se pudo conectar con MySQL";

            return crow::response(503, response);
        }

        try {
            const auto vehiculos = db.selectPrepared(
                "SELECT IdVehiculo, Placa, TipoVehiculo, "
                "FechaRegistro, EstadoRegistro "
                "FROM Vehiculos "
                "WHERE IdVehiculo = ? "
                "AND EstadoRegistro = 'A'",
                {std::to_string(idVehiculo)}
            );

            if (vehiculos.empty()) {
                response["success"] = false;
                response["message"] =
                    "Vehiculo no encontrado";

                return crow::response(404, response);
            }

            response["success"] = true;
            response["message"] =
                "Vehiculo consultado correctamente";

            response["data"] =
                mapVehiculo(vehiculos.front());

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr << "Error consultando vehiculo: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["message"] =
                "Error al consultar el vehiculo";

            return crow::response(500, response);
        }
    });


    // =====================================================
    // ENDPOINT: REGISTRAR VEHICULO
    // =====================================================

    CROW_ROUTE(app, "/api/vehiculos")
    .methods(crow::HTTPMethod::POST)
    ([](const crow::request& request) {
        crow::json::wvalue response;

        const auto body = crow::json::load(request.body);

        if (!body ||
            !body.has("placa") ||
            !body.has("tipoVehiculo") ||
            body["placa"].t() != crow::json::type::String ||
            body["tipoVehiculo"].t() != crow::json::type::String) {

            response["success"] = false;
            response["message"] =
                "Debe enviar placa y tipoVehiculo como texto";

            return crow::response(400, response);
        }

        const std::string placa = normalizePlaca(
            std::string(body["placa"].s())
        );

        const std::string tipoVehiculo = trim(
            std::string(body["tipoVehiculo"].s())
        );

        if (!isValidPlaca(placa)) {
            response["success"] = false;
            response["message"] =
                "La placa debe contener entre 5 y 10 caracteres alfanumericos";

            return crow::response(400, response);
        }

        if (!isValidTipoVehiculo(tipoVehiculo)) {
            response["success"] = false;
            response["message"] =
                "Tipo de vehiculo invalido. Valores permitidos: "
                "Automovil, Motocicleta, Camion o Bus";

            return crow::response(400, response);
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            response["success"] = false;
            response["message"] =
                "No se pudo conectar con MySQL";

            return crow::response(503, response);
        }

        try {
            const auto existentes = db.selectPrepared(
                "SELECT IdVehiculo "
                "FROM Vehiculos "
                "WHERE Placa = ? "
                "LIMIT 1",
                {placa}
            );

            if (!existentes.empty()) {
                response["success"] = false;
                response["message"] =
                    "Ya existe un vehiculo registrado con esta placa";

                return crow::response(409, response);
            }

            const unsigned long long idVehiculo = db.insert(
                "INSERT INTO Vehiculos "
                "(Placa, TipoVehiculo, FechaRegistro, EstadoRegistro) "
                "VALUES (?, ?, NOW(), 'A')",
                {placa, tipoVehiculo}
            );

            response["success"] = true;
            response["message"] =
                "Vehiculo registrado correctamente";

            response["data"]["idVehiculo"] =
                static_cast<std::uint64_t>(idVehiculo);

            response["data"]["placa"] = placa;
            response["data"]["tipoVehiculo"] = tipoVehiculo;
            response["data"]["estadoRegistro"] = "A";

            return crow::response(201, response);

        } catch (const std::exception& error) {
            std::cerr << "Error registrando vehiculo: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["message"] =
                "Ocurrio un error al registrar el vehiculo";

            return crow::response(500, response);
        }
    });


}