#include "routes/SimulacionRoutes.h"
#include "database/DatabaseConnection.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <utility>

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
            std::size_t position = 0;
            const unsigned long port =
                std::stoul(portText, &position);

            if (position != portText.size() ||
                port == 0 || port > 65535) {
                return false;
            }

            return db.connect(
                host,
                user,
                password,
                database,
                static_cast<unsigned int>(port)
            );

        } catch (const std::exception&) {
            return false;
        }
    }

    bool readPositiveId(
        const crow::json::rvalue& body,
        const char* field,
        int& value
    ) {
        if (!body.has(field) ||
            body[field].t() != crow::json::type::Number) {
            return false;
        }

        const double number = body[field].d();

        if (!std::isfinite(number) ||
            number < 1 ||
            number > std::numeric_limits<int>::max() ||
            std::floor(number) != number) {
            return false;
        }

        value = static_cast<int>(number);
        return true;
    }

    bool readNonNegativeInteger(
        const crow::json::rvalue& body,
        const char* field,
        int& value
    ) {
        if (!body.has(field) ||
            body[field].t() != crow::json::type::Number) {
            return false;
        }

        const double number = body[field].d();

        if (!std::isfinite(number) ||
            number < 0 ||
            number > std::numeric_limits<int>::max() ||
            std::floor(number) != number) {
            return false;
        }

        value = static_cast<int>(number);
        return true;
    }

    bool readSpeed(
        const crow::json::rvalue& body,
        double& value
    ) {
        if (!body.has("velocidad") ||
            body["velocidad"].t() !=
                crow::json::type::Number) {
            return false;
        }

        value = body["velocidad"].d();

        return std::isfinite(value) &&
               value >= 0.0 &&
               value <= 9999.99;
    }

    std::string formatSpeed(double speed) {
        // La columna MySQL es DECIMAL(6,2).
        const long long hundredths =
            std::llround(speed * 100.0);

        const long long integerPart =
            hundredths / 100;

        const long long decimalPart =
            hundredths % 100;

        std::string decimal =
            std::to_string(decimalPart);

        if (decimalPart < 10) {
            decimal = "0" + decimal;
        }

        return std::to_string(integerPart) +
               "." + decimal;
    }

}

void registerSimulacionRoutes(crow::SimpleApp& app) {

    // =====================================================
    // ASIGNAR VEHICULO A UNA SIMULACION
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>/vehiculos")
    .methods(crow::HTTPMethod::POST)
    ([](const crow::request& request, int idSimulacion) {
        crow::json::wvalue response;

        if (idSimulacion <= 0) {
            response["success"] = false;
            response["message"] =
                "El identificador de simulacion es invalido";

            return crow::response(400, response);
        }

        const auto body = crow::json::load(request.body);

        if (!body || body.t() != crow::json::type::Object) {
            response["success"] = false;
            response["message"] =
                "Debe enviar un objeto JSON valido";

            return crow::response(400, response);
        }

        int idVehiculo = 0;
        int idCalle = 0;
        int segundoLlegada = 0;
        double velocidad = 0.0;

        if (!readPositiveId(body, "idVehiculo", idVehiculo) ||
            !readPositiveId(body, "idCalle", idCalle) ||
            !readNonNegativeInteger(
                body,
                "segundoLlegada",
                segundoLlegada
            ) ||
            !readSpeed(body, velocidad)) {

            response["success"] = false;
            response["message"] =
                "Datos invalidos. Verifique idVehiculo, "
                "idCalle, velocidad y segundoLlegada";

            return crow::response(400, response);
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            response["success"] = false;
            response["message"] =
                "No se pudo conectar con MySQL";

            return crow::response(503, response);
        }

        bool transactionStarted = false;

        try {
            if (!db.beginTransaction()) {
                throw std::runtime_error(
                    "No se pudo iniciar la transaccion"
                );
            }

            transactionStarted = true;

            // Bloquear la simulacion durante el registro.
            const auto simulaciones = db.selectPrepared(
                "SELECT Estado, DuracionSimuladaSegundos, "
                "HoraSimuladaInicio "
                "FROM Simulaciones "
                "WHERE IdSimulacion = ? "
                "FOR UPDATE",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                response["success"] = false;
                response["message"] =
                    "La simulacion no existe";

                return crow::response(404, response);
            }

            const auto& simulacion = simulaciones.front();

            if (simulacion.at("Estado") != "Configurada") {
                db.rollbackTransaction();
                transactionStarted = false;

                response["success"] = false;
                response["message"] =
                    "Solo se pueden agregar vehiculos "
                    "a simulaciones configuradas";

                return crow::response(409, response);
            }

            const int duracion = std::stoi(
                simulacion.at("DuracionSimuladaSegundos")
            );

            if (segundoLlegada >= duracion) {
                db.rollbackTransaction();
                transactionStarted = false;

                response["success"] = false;
                response["message"] =
                    "El segundo de llegada debe ser menor "
                    "que la duracion de la simulacion";

                return crow::response(400, response);
            }

            // Verificar vehiculo activo.
            const auto vehiculos = db.selectPrepared(
                "SELECT IdVehiculo "
                "FROM Vehiculos "
                "WHERE IdVehiculo = ? "
                "AND EstadoRegistro = 'A'",
                {std::to_string(idVehiculo)}
            );

            if (vehiculos.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                response["success"] = false;
                response["message"] =
                    "El vehiculo no existe o esta inactivo";

                return crow::response(404, response);
            }

            // Verificar calle activa.
            const auto calles = db.selectPrepared(
                "SELECT IdCalle "
                "FROM Calles "
                "WHERE IdCalle = ? "
                "AND EstadoRegistro = 'A'",
                {std::to_string(idCalle)}
            );

            if (calles.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                response["success"] = false;
                response["message"] =
                    "La calle no existe o esta inactiva";

                return crow::response(404, response);
            }

            // Evitar duplicados en la misma simulacion.
            const auto existentes = db.selectPrepared(
                "SELECT IdSimulacionVehiculo "
                "FROM SimulacionVehiculos "
                "WHERE IdSimulacion = ? "
                "AND IdVehiculo = ?",
                {
                    std::to_string(idSimulacion),
                    std::to_string(idVehiculo)
                }
            );

            if (!existentes.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                response["success"] = false;
                response["message"] =
                    "El vehiculo ya esta registrado "
                    "en esta simulacion";

                return crow::response(409, response);
            }

            // MySQL calcula la hora de llegada.
            const unsigned long long idRegistro = db.insert(
                "INSERT INTO SimulacionVehiculos "
                "(IdSimulacion, IdVehiculo, IdCalle, "
                "Velocidad, HoraLlegada, SegundoLlegada, "
                "TiempoEsperaSegundos, Estado) "
                "VALUES (?, ?, ?, ?, "
                "ADDTIME(?, SEC_TO_TIME(?)), ?, 0, 'Programado')",
                {
                    std::to_string(idSimulacion),
                    std::to_string(idVehiculo),
                    std::to_string(idCalle),
                    formatSpeed(velocidad),
                    simulacion.at("HoraSimuladaInicio"),
                    std::to_string(segundoLlegada),
                    std::to_string(segundoLlegada)
                }
            );

            // Actualizar el contador de la simulacion.
            const unsigned long long affectedRows =
                db.executePrepared(
                    "UPDATE Simulaciones "
                    "SET TotalVehiculos = TotalVehiculos + 1 "
                    "WHERE IdSimulacion = ?",
                    {std::to_string(idSimulacion)}
                );

            if (affectedRows != 1) {
                throw std::runtime_error(
                    "No se pudo actualizar TotalVehiculos"
                );
            }

            if (!db.commitTransaction()) {
                throw std::runtime_error(
                    "No se pudo confirmar la transaccion"
                );
            }

            transactionStarted = false;

            response["success"] = true;
            response["message"] =
                "Vehiculo agregado a la simulacion correctamente";

            response["data"]["idSimulacionVehiculo"] =
                static_cast<std::uint64_t>(idRegistro);

            response["data"]["idSimulacion"] =
                idSimulacion;

            response["data"]["idVehiculo"] =
                idVehiculo;

            response["data"]["idCalle"] =
                idCalle;

            response["data"]["velocidad"] =
                velocidad;

            response["data"]["segundoLlegada"] =
                segundoLlegada;

            response["data"]["estado"] =
                "Programado";

            return crow::response(201, response);

        } catch (const std::exception& error) {
            if (transactionStarted) {
                db.rollbackTransaction();
            }

            std::cerr << "Error agregando vehiculo: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["message"] =
                "Ocurrio un error al agregar el vehiculo";

            return crow::response(500, response);
        }
    });


    // =====================================================
    // ENDPOINT: CONSULTAR VEHICULOS DE UNA SIMULACION
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>/vehiculos")
    .methods(crow::HTTPMethod::GET)
    ([](int idSimulacion) {
        crow::json::wvalue response;

        if (idSimulacion <= 0) {
            response["success"] = false;
            response["message"] =
                "El identificador de simulacion es invalido";

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
            // Verificar que la simulacion exista.
            const auto simulaciones = db.selectPrepared(
                "SELECT IdSimulacion, Nombre, Estado "
                "FROM Simulaciones "
                "WHERE IdSimulacion = ?",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                response["success"] = false;
                response["message"] =
                    "La simulacion no existe";

                return crow::response(404, response);
            }

            // Consultar los vehiculos asociados.
            const auto vehiculos = db.selectPrepared(
                "SELECT "
                "sv.IdSimulacionVehiculo, "
                "sv.IdVehiculo, "
                "v.Placa, "
                "v.TipoVehiculo, "
                "sv.IdCalle, "
                "c.Nombre AS Calle, "
                "sv.Velocidad, "
                "sv.HoraLlegada, "
                "sv.HoraInicioCruce, "
                "sv.HoraSalida, "
                "sv.SegundoLlegada, "
                "sv.SegundoInicioCruce, "
                "sv.SegundoSalida, "
                "sv.TiempoEsperaSegundos, "
                "sv.Estado "
                "FROM SimulacionVehiculos sv "
                "INNER JOIN Vehiculos v "
                "ON sv.IdVehiculo = v.IdVehiculo "
                "INNER JOIN Calles c "
                "ON sv.IdCalle = c.IdCalle "
                "WHERE sv.IdSimulacion = ? "
                "ORDER BY sv.SegundoLlegada, "
                "sv.IdSimulacionVehiculo",
                {std::to_string(idSimulacion)}
            );

            crow::json::wvalue::list listaVehiculos;

            for (const auto& vehiculo : vehiculos) {
                crow::json::wvalue item;

                item["idSimulacionVehiculo"] =
                    std::stoi(
                        vehiculo.at("IdSimulacionVehiculo")
                    );

                item["idVehiculo"] =
                    std::stoi(vehiculo.at("IdVehiculo"));

                item["placa"] =
                    vehiculo.at("Placa");

                item["tipoVehiculo"] =
                    vehiculo.at("TipoVehiculo");

                item["idCalle"] =
                    std::stoi(vehiculo.at("IdCalle"));

                item["calle"] =
                    vehiculo.at("Calle");

                item["velocidad"] =
                    std::stod(vehiculo.at("Velocidad"));

                item["horaLlegada"] =
                    vehiculo.at("HoraLlegada");

                item["segundoLlegada"] =
                    std::stoi(
                        vehiculo.at("SegundoLlegada")
                    );

                item["tiempoEsperaSegundos"] =
                    std::stoi(
                        vehiculo.at("TiempoEsperaSegundos")
                    );

                item["estado"] =
                    vehiculo.at("Estado");

                // Los campos de cruce y salida pueden ser NULL.
                const auto& segundoInicio =
                    vehiculo.at("SegundoInicioCruce");

                const auto& segundoSalida =
                    vehiculo.at("SegundoSalida");

                const auto& horaInicio =
                    vehiculo.at("HoraInicioCruce");

                const auto& horaSalida =
                    vehiculo.at("HoraSalida");

                if (segundoInicio.empty()) {
                    item["segundoInicioCruce"] = nullptr;
                } else {
                    item["segundoInicioCruce"] =
                        std::stoi(segundoInicio);
                }

                if (segundoSalida.empty()) {
                    item["segundoSalida"] = nullptr;
                } else {
                    item["segundoSalida"] =
                        std::stoi(segundoSalida);
                }

                if (horaInicio.empty()) {
                    item["horaInicioCruce"] = nullptr;
                } else {
                    item["horaInicioCruce"] = horaInicio;
                }

                if (horaSalida.empty()) {
                    item["horaSalida"] = nullptr;
                } else {
                    item["horaSalida"] = horaSalida;
                }

                listaVehiculos.push_back(
                    std::move(item)
                );
            }

            response["success"] = true;
            response["message"] =
                "Vehiculos de la simulacion consultados correctamente";

            response["idSimulacion"] =
                idSimulacion;

            response["nombreSimulacion"] =
                simulaciones.front().at("Nombre");

            response["estadoSimulacion"] =
                simulaciones.front().at("Estado");

            response["total"] =
                static_cast<int>(vehiculos.size());

            response["data"] =
                std::move(listaVehiculos);

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr << "Error consultando vehiculos de simulacion: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["message"] =
                "Error al consultar vehiculos de la simulacion";

            return crow::response(500, response);
        }
    });


    // =====================================================
    // ENDPOINT: CONSULTAR TODAS LAS SIMULACIONES
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones")
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
            const auto simulaciones = db.select(
                "SELECT "
                "IdSimulacion, "
                "Nombre, "
                "Descripcion, "
                "FechaCreacion, "
                "FechaInicio, "
                "FechaFin, "
                "HoraSimuladaInicio, "
                "DuracionSimuladaSegundos, "
                "Estado, "
                "TotalVehiculos, "
                "TotalVehiculosFinalizados "
                "FROM Simulaciones "
                "ORDER BY IdSimulacion DESC"
            );

            crow::json::wvalue::list listaSimulaciones;

            for (const auto& simulacion : simulaciones) {
                crow::json::wvalue item;

                item["idSimulacion"] =
                    std::stoi(simulacion.at("IdSimulacion"));

                item["nombre"] =
                    simulacion.at("Nombre");

                item["descripcion"] =
                    simulacion.at("Descripcion");

                item["fechaCreacion"] =
                    simulacion.at("FechaCreacion");

                const auto& fechaInicio =
                    simulacion.at("FechaInicio");

                const auto& fechaFin =
                    simulacion.at("FechaFin");

                if (fechaInicio.empty()) {
                    item["fechaInicio"] = nullptr;
                } else {
                    item["fechaInicio"] = fechaInicio;
                }

                if (fechaFin.empty()) {
                    item["fechaFin"] = nullptr;
                } else {
                    item["fechaFin"] = fechaFin;
                }

                item["horaSimuladaInicio"] =
                    simulacion.at("HoraSimuladaInicio");

                item["duracionSimuladaSegundos"] =
                    std::stoi(
                        simulacion.at("DuracionSimuladaSegundos")
                    );

                item["estado"] =
                    simulacion.at("Estado");

                item["totalVehiculos"] =
                    std::stoi(
                        simulacion.at("TotalVehiculos")
                    );

                item["totalVehiculosFinalizados"] =
                    std::stoi(
                        simulacion.at("TotalVehiculosFinalizados")
                    );

                listaSimulaciones.push_back(
                    std::move(item)
                );
            }

            response["success"] = true;
            response["message"] =
                "Simulaciones consultadas correctamente";

            response["total"] =
                static_cast<int>(simulaciones.size());

            response["data"] =
                std::move(listaSimulaciones);

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr << "Error consultando simulaciones: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["message"] =
                "Error al consultar las simulaciones";

            return crow::response(500, response);
        }
    });


    // =====================================================
    // ENDPOINT: CONSULTAR SIMULACION POR ID
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>")
    .methods(crow::HTTPMethod::GET)
    ([](int idSimulacion) {
        crow::json::wvalue response;

        if (idSimulacion <= 0) {
            response["success"] = false;
            response["message"] =
                "El identificador de simulacion es invalido";

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
            const auto simulaciones = db.selectPrepared(
                "SELECT "
                "IdSimulacion, "
                "Nombre, "
                "Descripcion, "
                "FechaCreacion, "
                "FechaInicio, "
                "FechaFin, "
                "HoraSimuladaInicio, "
                "DuracionSimuladaSegundos, "
                "Estado, "
                "TotalVehiculos, "
                "TotalVehiculosFinalizados "
                "FROM Simulaciones "
                "WHERE IdSimulacion = ?",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                response["success"] = false;
                response["message"] =
                    "La simulacion no existe";

                return crow::response(404, response);
            }

            const auto& simulacion = simulaciones.front();

            crow::json::wvalue item;

            item["idSimulacion"] =
                std::stoi(simulacion.at("IdSimulacion"));

            item["nombre"] =
                simulacion.at("Nombre");

            item["descripcion"] =
                simulacion.at("Descripcion");

            item["fechaCreacion"] =
                simulacion.at("FechaCreacion");

            const auto& fechaInicio =
                simulacion.at("FechaInicio");

            const auto& fechaFin =
                simulacion.at("FechaFin");

            if (fechaInicio.empty()) {
                item["fechaInicio"] = nullptr;
            } else {
                item["fechaInicio"] = fechaInicio;
            }

            if (fechaFin.empty()) {
                item["fechaFin"] = nullptr;
            } else {
                item["fechaFin"] = fechaFin;
            }

            item["horaSimuladaInicio"] =
                simulacion.at("HoraSimuladaInicio");

            item["duracionSimuladaSegundos"] =
                std::stoi(
                    simulacion.at("DuracionSimuladaSegundos")
                );

            item["estado"] =
                simulacion.at("Estado");

            item["totalVehiculos"] =
                std::stoi(
                    simulacion.at("TotalVehiculos")
                );

            item["totalVehiculosFinalizados"] =
                std::stoi(
                    simulacion.at("TotalVehiculosFinalizados")
                );

            response["success"] = true;
            response["message"] =
                "Simulacion consultada correctamente";

            response["data"] = std::move(item);

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr << "Error consultando simulacion: "
                      << error.what()
                      << std::endl;

            response["success"] = false;
            response["message"] =
                "Error al consultar la simulacion";

            return crow::response(500, response);
        }
    });


}
