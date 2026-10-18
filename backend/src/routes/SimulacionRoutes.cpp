#include "routes/SimulacionRoutes.h"
#include "database/DatabaseConnection.h"
#include "simulation/SimulationEngine.h"
#include "statistics/StatisticsService.h"
#include "traffic/SpeedControlService.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

    std::mutex simulationMutex;
    std::map<int, SimulationEngine> activeSimulations;

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
                host, user, password, database,
                static_cast<unsigned int>(port)
            );
        } catch (const std::exception&) {
            return false;
        }
    }

    bool readInteger(
        const crow::json::rvalue& body,
        const char* field,
        int& value,
        int minimum,
        int maximum
    ) {
        if (!body.has(field) ||
            body[field].t() != crow::json::type::Number) {
            return false;
        }

        const double number = body[field].d();

        if (!std::isfinite(number) ||
            number < minimum ||
            number > maximum ||
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
        const long long hundredths =
            std::llround(speed * 100.0);

        std::string decimals =
            std::to_string(hundredths % 100);

        if (hundredths % 100 < 10) {
            decimals = "0" + decimals;
        }

        return std::to_string(hundredths / 100) +
               "." + decimals;
    }

    crow::response errorResponse(
        int status,
        const std::string& message
    ) {
        crow::json::wvalue response;
        response["success"] = false;
        response["message"] = message;
        return crow::response(status, response);
    }

    std::string getValue(
        const std::map<std::string, std::string>& row,
        const std::string& column
    ) {
        const auto it = row.find(column);

        if (it == row.end()) {
            throw std::runtime_error(
                "Columna no encontrada: " + column
            );
        }

        return it->second;
    }

    void setNullableString(
        crow::json::wvalue& item,
        const std::string& key,
        const std::string& value
    ) {
        if (value.empty()) {
            item[key] = nullptr;
        } else {
            item[key] = value;
        }
    }

    void setNullableInteger(
        crow::json::wvalue& item,
        const std::string& key,
        const std::string& value
    ) {
        if (value.empty()) {
            item[key] = nullptr;
        } else {
            item[key] = std::stoi(value);
        }
    }

    crow::json::wvalue simulationToJson(
        const std::map<std::string, std::string>& row
    ) {
        crow::json::wvalue item;

        item["idSimulacion"] =
            std::stoi(getValue(row, "IdSimulacion"));
        item["nombre"] = getValue(row, "Nombre");

        setNullableString(
            item, "descripcion",
            getValue(row, "Descripcion")
        );

        item["fechaCreacion"] =
            getValue(row, "FechaCreacion");

        setNullableString(
            item, "fechaInicio",
            getValue(row, "FechaInicio")
        );

        setNullableString(
            item, "fechaFin",
            getValue(row, "FechaFin")
        );

        item["horaSimuladaInicio"] =
            getValue(row, "HoraSimuladaInicio");

        item["duracionSimuladaSegundos"] =
            std::stoi(
                getValue(row, "DuracionSimuladaSegundos")
            );

        item["estado"] = getValue(row, "Estado");

        item["totalVehiculos"] =
            std::stoi(getValue(row, "TotalVehiculos"));

        item["totalVehiculosFinalizados"] =
            std::stoi(
                getValue(row, "TotalVehiculosFinalizados")
            );

        return item;
    }

    const std::string simulationColumns =
        "IdSimulacion, Nombre, Descripcion, "
        "FechaCreacion, FechaInicio, FechaFin, "
        "HoraSimuladaInicio, DuracionSimuladaSegundos, "
        "Estado, TotalVehiculos, TotalVehiculosFinalizados ";

    crow::json::wvalue vehicleToJson(
        const std::map<std::string, std::string>& row
    ) {
        crow::json::wvalue item;

        item["idSimulacionVehiculo"] =
            std::stoi(
                getValue(row, "IdSimulacionVehiculo")
            );

        item["idVehiculo"] =
            std::stoi(getValue(row, "IdVehiculo"));

        item["placa"] = getValue(row, "Placa");
        item["tipoVehiculo"] =
            getValue(row, "TipoVehiculo");

        item["idCalle"] =
            std::stoi(getValue(row, "IdCalle"));

        item["calle"] = getValue(row, "Calle");

        item["velocidad"] =
            std::stod(getValue(row, "Velocidad"));

        item["horaLlegada"] =
            getValue(row, "HoraLlegada");

        item["segundoLlegada"] =
            std::stoi(getValue(row, "SegundoLlegada"));

        item["tiempoEsperaSegundos"] =
            std::stoi(
                getValue(row, "TiempoEsperaSegundos")
            );

        item["estado"] = getValue(row, "Estado");

        setNullableInteger(
            item, "segundoInicioCruce",
            getValue(row, "SegundoInicioCruce")
        );

        setNullableInteger(
            item, "segundoSalida",
            getValue(row, "SegundoSalida")
        );

        setNullableString(
            item, "horaInicioCruce",
            getValue(row, "HoraInicioCruce")
        );

        setNullableString(
            item, "horaSalida",
            getValue(row, "HoraSalida")
        );

        return item;
    }

    crow::json::wvalue summaryToJson(
        int idSimulacion,
        const SimulationEngine& engine
    ) {
        const auto resumen = engine.obtenerResumen();

        crow::json::wvalue response;

        response["success"] = true;
        response["idSimulacion"] = idSimulacion;
        response["segundoActual"] = resumen.segundoActual;
        response["duracionSegundos"] =
            engine.obtenerDuracionSegundos();

        response["totalVehiculos"] =
            resumen.totalVehiculos;
        response["programados"] = resumen.programados;
        response["enCola"] = resumen.enCola;
        response["cruzando"] = resumen.cruzando;
        response["finalizados"] = resumen.finalizados;

        crow::json::wvalue::list colas;

        for (int cantidad : resumen.vehiculosEnCola) {
            colas.emplace_back(cantidad);
        }

        response["colasPorCalle"] = std::move(colas);

        // FASE 6: Estado de los semaforos inteligentes.
        const auto& semaforos = resumen.semaforos;
        crow::json::wvalue semaforosJson;
        semaforosJson["segundoActual"] = semaforos.segundoActual;
        semaforosJson["calleConPrioridad"] = semaforos.calleConPrioridad;
        semaforosJson["segundosRestantes"] = semaforos.segundosRestantes;

        const std::array<std::string, 4> nombresCalles = {
            "Norte", "Sur", "Este", "Oeste"
        };
        crow::json::wvalue::list calles;
        for (int i = 0; i < 4; ++i) {
            crow::json::wvalue calle;
            calle["idCalle"] = i + 1;
            calle["nombre"] = nombresCalles[i];
            calle["estado"] = TrafficLightController::estadoComoTexto(
                semaforos.estados[i]
            );
            calle["vehiculosEnCola"] = semaforos.vehiculosEnCola[i];
            calles.push_back(std::move(calle));
        }
        semaforosJson["calles"] = std::move(calles);
        response["semaforos"] = std::move(semaforosJson);

        return response;
    }

    // Guarda el resultado del motor utilizando las columnas
    // existentes en SimulacionVehiculos y Simulaciones.
    //
    // Debe llamarse dentro de una transaccion activa.
    void persistSimulation(
        DatabaseConnection& db,
        int idSimulacion,
        const SimulationEngine& engine,
        const std::string& horaBase
    ) {
        for (const auto& vehiculo :
             engine.obtenerVehiculos()) {

            const std::string inicio =
                std::to_string(
                    vehiculo.segundoInicioCruce
                );

            const std::string salida =
                std::to_string(
                    vehiculo.segundoSalida
                );

            db.executePrepared(
                "UPDATE SimulacionVehiculos SET "
                "Estado = ?, "
                "SegundoInicioCruce = "
                "IF(? = '-1', NULL, CAST(? AS SIGNED)), "
                "SegundoSalida = "
                "IF(? = '-1', NULL, CAST(? AS SIGNED)), "
                "HoraInicioCruce = "
                "IF(? = '-1', NULL, "
                "ADDTIME(?, SEC_TO_TIME(?))), "
                "HoraSalida = "
                "IF(? = '-1', NULL, "
                "ADDTIME(?, SEC_TO_TIME(?))), "
                "TiempoEsperaSegundos = ? "
                "WHERE IdSimulacionVehiculo = ? "
                "AND IdSimulacion = ?",
                {
                    SimulationEngine::estadoComoTexto(
                        vehiculo.estado
                    ),
                    inicio,
                    inicio,
                    salida,
                    salida,
                    inicio,
                    horaBase,
                    inicio,
                    salida,
                    horaBase,
                    salida,
                    std::to_string(
                        vehiculo.tiempoEsperaSegundos
                    ),
                    std::to_string(
                        vehiculo.idSimulacionVehiculo
                    ),
                    std::to_string(idSimulacion)
                }
            );
        }

        const auto resumen = engine.obtenerResumen();

        if (engine.haFinalizado()) {
            db.executePrepared(
                "UPDATE Simulaciones SET "
                "TotalVehiculosFinalizados = ?, "
                "Estado = 'Finalizada', "
                "FechaFin = NOW() "
                "WHERE IdSimulacion = ?",
                {
                    std::to_string(resumen.finalizados),
                    std::to_string(idSimulacion)
                }
            );
        } else {
            db.executePrepared(
                "UPDATE Simulaciones SET "
                "TotalVehiculosFinalizados = ? "
                "WHERE IdSimulacion = ?",
                {
                    std::to_string(resumen.finalizados),
                    std::to_string(idSimulacion)
                }
            );
        }
    }

    // Ejecuta uno o varios segundos con una sola
    // transaccion y una sola persistencia final.
    crow::response advanceSimulation(
        int idSimulacion,
        int segundos
    ) {
        if (idSimulacion <= 0) {
            return errorResponse(
                400, "Identificador invalido"
            );
        }

        if (segundos < 1 || segundos > 3600) {
            return errorResponse(
                400,
                "Los segundos deben estar entre 1 y 3600"
            );
        }

        std::lock_guard<std::mutex> lock(simulationMutex);

        auto it = activeSimulations.find(idSimulacion);

        if (it == activeSimulations.end()) {
            return errorResponse(
                409, "Simulacion no activa en memoria"
            );
        }

        if (it->second.haFinalizado()) {
            return errorResponse(
                409, "La simulacion ya termino"
            );
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            return errorResponse(
                503, "No se pudo conectar con MySQL"
            );
        }

        bool transactionStarted = false;

        try {
            // No modificar el motor original hasta
            // confirmar la transaccion.
            SimulationEngine siguiente = it->second;

            const int segundoAnterior =
                siguiente.obtenerSegundoActual();

            int segundosAvanzados = 0;

            for (int i = 0; i < segundos; i++) {
                if (!siguiente.avanzarSegundo()) {
                    break;
                }

                segundosAvanzados++;

                if (siguiente.haFinalizado()) {
                    break;
                }
            }

            if (segundosAvanzados == 0) {
                return errorResponse(
                    409, "No quedan segundos por avanzar"
                );
            }

            if (!db.beginTransaction()) {
                throw std::runtime_error(
                    "No se pudo iniciar la transaccion"
                );
            }

            transactionStarted = true;

            const auto simulaciones = db.selectPrepared(
                "SELECT Estado, HoraSimuladaInicio "
                "FROM Simulaciones "
                "WHERE IdSimulacion = ? "
                "FOR UPDATE",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    404, "La simulacion no existe"
                );
            }

            if (getValue(
                    simulaciones.front(), "Estado"
                ) != "Ejecutando") {

                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    409,
                    "La simulacion no esta en estado Ejecutando"
                );
            }

            const std::string horaBase =
                getValue(
                    simulaciones.front(),
                    "HoraSimuladaInicio"
                );

            persistSimulation(
                db,
                idSimulacion,
                siguiente,
                horaBase
            );

            // Registrar infracciones del intervalo avanzado
            // dentro de la misma transaccion.
            const int multasRegistradas =
                SpeedControlService::registrarMultas(
                    db,
                    idSimulacion,
                    segundoAnterior,
                    siguiente.obtenerSegundoActual()
                );

            if (!db.commitTransaction()) {
                throw std::runtime_error(
                    "No se pudo confirmar la transaccion"
                );
            }

            transactionStarted = false;

            const bool terminada =
                siguiente.haFinalizado();

            it->second = std::move(siguiente);

            crow::json::wvalue response =
                summaryToJson(
                    idSimulacion,
                    it->second
                );

            response["message"] =
                "Simulacion avanzada correctamente";

            response["segundosAvanzados"] =
                segundosAvanzados;

            response["multasRegistradas"] =
                multasRegistradas;

            response["estadoSimulacion"] =
                terminada ? "Finalizada" : "Ejecutando";

            if (terminada) {
                activeSimulations.erase(it);
            }

            return crow::response(200, response);

        } catch (const std::exception& error) {
            if (transactionStarted) {
                db.rollbackTransaction();
            }

            std::cerr
                << "Error avanzando simulacion: "
                << error.what()
                << std::endl;

            return errorResponse(
                500, "Error al avanzar la simulacion"
            );
        }
    }

}

void registerSimulacionRoutes(crow::App<crow::CORSHandler>& app) {

    // =====================================================
    // FASE 4: ASIGNAR VEHICULO A SIMULACION
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>/vehiculos")
    .methods(crow::HTTPMethod::POST)
    ([](const crow::request& request, int idSimulacion) {
        if (idSimulacion <= 0) {
            return errorResponse(
                400, "Identificador de simulacion invalido"
            );
        }

        const auto body = crow::json::load(request.body);

        if (!body ||
            body.t() != crow::json::type::Object) {
            return errorResponse(
                400, "Debe enviar un objeto JSON valido"
            );
        }

        int idVehiculo = 0;
        int idCalle = 0;
        int segundoLlegada = 0;
        double velocidad = 0.0;

        if (!readInteger(
                body, "idVehiculo", idVehiculo,
                1, std::numeric_limits<int>::max()
            ) ||
            !readInteger(
                body, "idCalle", idCalle,
                1, std::numeric_limits<int>::max()
            ) ||
            !readInteger(
                body, "segundoLlegada", segundoLlegada,
                0, std::numeric_limits<int>::max()
            ) ||
            !readSpeed(body, velocidad)) {

            return errorResponse(
                400,
                "Datos invalidos. Verifique idVehiculo, "
                "idCalle, velocidad y segundoLlegada"
            );
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            return errorResponse(
                503, "No se pudo conectar con MySQL"
            );
        }

        bool transactionStarted = false;

        try {
            if (!db.beginTransaction()) {
                throw std::runtime_error(
                    "No se pudo iniciar la transaccion"
                );
            }

            transactionStarted = true;

            const auto simulaciones = db.selectPrepared(
                "SELECT Estado, DuracionSimuladaSegundos, "
                "HoraSimuladaInicio "
                "FROM Simulaciones "
                "WHERE IdSimulacion = ? FOR UPDATE",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    404, "La simulacion no existe"
                );
            }

            const auto& simulacion =
                simulaciones.front();

            if (getValue(
                    simulacion, "Estado"
                ) != "Configurada") {

                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    409,
                    "Solo se pueden agregar vehiculos "
                    "a simulaciones configuradas"
                );
            }

            const int duracion = std::stoi(
                getValue(
                    simulacion,
                    "DuracionSimuladaSegundos"
                )
            );

            if (segundoLlegada >= duracion) {
                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    400,
                    "El segundo de llegada debe ser menor "
                    "que la duracion de la simulacion"
                );
            }

            const auto vehiculos = db.selectPrepared(
                "SELECT IdVehiculo FROM Vehiculos "
                "WHERE IdVehiculo = ? "
                "AND EstadoRegistro = 'A'",
                {std::to_string(idVehiculo)}
            );

            if (vehiculos.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    404,
                    "El vehiculo no existe o esta inactivo"
                );
            }

            const auto calles = db.selectPrepared(
                "SELECT IdCalle FROM Calles "
                "WHERE IdCalle = ? "
                "AND EstadoRegistro = 'A'",
                {std::to_string(idCalle)}
            );

            if (calles.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    404,
                    "La calle no existe o esta inactiva"
                );
            }

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

                return errorResponse(
                    409,
                    "El vehiculo ya esta registrado "
                    "en esta simulacion"
                );
            }

            const unsigned long long idRegistro =
                db.insert(
                    "INSERT INTO SimulacionVehiculos "
                    "(IdSimulacion, IdVehiculo, IdCalle, "
                    "Velocidad, HoraLlegada, SegundoLlegada, "
                    "TiempoEsperaSegundos, Estado) "
                    "VALUES (?, ?, ?, ?, "
                    "ADDTIME(?, SEC_TO_TIME(?)), "
                    "?, 0, 'Programado')",
                    {
                        std::to_string(idSimulacion),
                        std::to_string(idVehiculo),
                        std::to_string(idCalle),
                        formatSpeed(velocidad),
                        getValue(
                            simulacion,
                            "HoraSimuladaInicio"
                        ),
                        std::to_string(segundoLlegada),
                        std::to_string(segundoLlegada)
                    }
                );

            const auto affectedRows =
                db.executePrepared(
                    "UPDATE Simulaciones "
                    "SET TotalVehiculos = "
                    "TotalVehiculos + 1 "
                    "WHERE IdSimulacion = ?",
                    {std::to_string(idSimulacion)}
                );

            if (affectedRows != 1) {
                throw std::runtime_error(
                    "No se actualizo TotalVehiculos"
                );
            }

            if (!db.commitTransaction()) {
                throw std::runtime_error(
                    "Error confirmando transaccion"
                );
            }

            transactionStarted = false;

            crow::json::wvalue response;

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

            std::cerr
                << "Error agregando vehiculo: "
                << error.what() << std::endl;

            return errorResponse(
                500, "Ocurrio un error al agregar el vehiculo"
            );
        }
    });

    // =====================================================
    // FASE 4: CONSULTAR VEHICULOS DE SIMULACION
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>/vehiculos")
    .methods(crow::HTTPMethod::GET)
    ([](int idSimulacion) {
        if (idSimulacion <= 0) {
            return errorResponse(
                400, "Identificador de simulacion invalido"
            );
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            return errorResponse(
                503, "No se pudo conectar con MySQL"
            );
        }

        try {
            const auto simulaciones = db.selectPrepared(
                "SELECT IdSimulacion, Nombre, Estado "
                "FROM Simulaciones "
                "WHERE IdSimulacion = ?",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                return errorResponse(
                    404, "La simulacion no existe"
                );
            }

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

            crow::json::wvalue::list lista;

            for (const auto& vehiculo : vehiculos) {
                lista.push_back(
                    vehicleToJson(vehiculo)
                );
            }

            crow::json::wvalue response;

            response["success"] = true;
            response["message"] =
                "Vehiculos de la simulacion consultados correctamente";
            response["idSimulacion"] = idSimulacion;
            response["nombreSimulacion"] =
                getValue(
                    simulaciones.front(), "Nombre"
                );
            response["estadoSimulacion"] =
                getValue(
                    simulaciones.front(), "Estado"
                );
            response["total"] =
                static_cast<int>(vehiculos.size());
            response["data"] = std::move(lista);

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr
                << "Error consultando vehiculos: "
                << error.what() << std::endl;

            return errorResponse(
                500,
                "Error al consultar vehiculos de la simulacion"
            );
        }
    });

    // =====================================================
    // FASE 4: CONSULTAR TODAS LAS SIMULACIONES
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones")
    .methods(crow::HTTPMethod::GET)
    ([]() {
        DatabaseConnection db;

        if (!connectDatabase(db)) {
            return errorResponse(
                503, "No se pudo conectar con MySQL"
            );
        }

        try {
            const auto simulaciones = db.select(
                "SELECT " + simulationColumns +
                "FROM Simulaciones "
                "ORDER BY IdSimulacion DESC"
            );

            crow::json::wvalue::list lista;

            for (const auto& simulacion : simulaciones) {
                lista.push_back(
                    simulationToJson(simulacion)
                );
            }

            crow::json::wvalue response;

            response["success"] = true;
            response["message"] =
                "Simulaciones consultadas correctamente";
            response["total"] =
                static_cast<int>(simulaciones.size());
            response["data"] = std::move(lista);

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr
                << "Error consultando simulaciones: "
                << error.what() << std::endl;

            return errorResponse(
                500, "Error al consultar las simulaciones"
            );
        }
    });

    // =====================================================
    // FASE 4: CONSULTAR SIMULACION POR ID
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>")
    .methods(crow::HTTPMethod::GET)
    ([](int idSimulacion) {
        if (idSimulacion <= 0) {
            return errorResponse(
                400, "Identificador de simulacion invalido"
            );
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            return errorResponse(
                503, "No se pudo conectar con MySQL"
            );
        }

        try {
            const auto simulaciones = db.selectPrepared(
                "SELECT " + simulationColumns +
                "FROM Simulaciones "
                "WHERE IdSimulacion = ?",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                return errorResponse(
                    404, "La simulacion no existe"
                );
            }

            crow::json::wvalue response;

            response["success"] = true;
            response["message"] =
                "Simulacion consultada correctamente";
            response["data"] =
                simulationToJson(
                    simulaciones.front()
                );

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr
                << "Error consultando simulacion: "
                << error.what() << std::endl;

            return errorResponse(
                500, "Error al consultar la simulacion"
            );
        }
    });

    // =====================================================
    // FASE 5: INICIAR SIMULACION
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>/iniciar")
    .methods(crow::HTTPMethod::POST)
    ([](int idSimulacion) {
        if (idSimulacion <= 0) {
            return errorResponse(
                400, "Identificador invalido"
            );
        }

        std::lock_guard<std::mutex> lock(
            simulationMutex
        );

        if (activeSimulations.contains(idSimulacion)) {
            return errorResponse(
                409,
                "La simulacion ya fue iniciada en este servidor"
            );
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            return errorResponse(
                503, "No se pudo conectar con MySQL"
            );
        }

        bool transactionStarted = false;

        try {
            if (!db.beginTransaction()) {
                throw std::runtime_error(
                    "Error iniciando transaccion"
                );
            }

            transactionStarted = true;

            const auto simulaciones = db.selectPrepared(
                "SELECT Estado, DuracionSimuladaSegundos "
                "FROM Simulaciones "
                "WHERE IdSimulacion = ? FOR UPDATE",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    404, "La simulacion no existe"
                );
            }

            if (getValue(
                    simulaciones.front(), "Estado"
                ) != "Configurada") {

                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    409,
                    "Solo se puede iniciar una simulacion Configurada"
                );
            }

            const int duracion = std::stoi(
                getValue(
                    simulaciones.front(),
                    "DuracionSimuladaSegundos"
                )
            );

            SimulationEngine engine(duracion);

            const auto vehiculos = db.selectPrepared(
                "SELECT "
                "sv.IdSimulacionVehiculo, "
                "sv.IdVehiculo, "
                "sv.IdCalle, "
                "sv.Velocidad, "
                "sv.SegundoLlegada, "
                "v.Placa "
                "FROM SimulacionVehiculos sv "
                "INNER JOIN Vehiculos v "
                "ON v.IdVehiculo = sv.IdVehiculo "
                "WHERE sv.IdSimulacion = ? "
                "ORDER BY sv.SegundoLlegada, "
                "sv.IdSimulacionVehiculo",
                {std::to_string(idSimulacion)}
            );

            if (vehiculos.empty()) {
                db.rollbackTransaction();
                transactionStarted = false;

                return errorResponse(
                    409,
                    "Agrega al menos un vehiculo antes de iniciar"
                );
            }

            for (const auto& row : vehiculos) {
                VehiculoSimulado vehiculo;

                vehiculo.idSimulacionVehiculo =
                    std::stoi(
                        getValue(
                            row,
                            "IdSimulacionVehiculo"
                        )
                    );

                vehiculo.idVehiculo =
                    std::stoi(
                        getValue(row, "IdVehiculo")
                    );

                vehiculo.idCalle =
                    std::stoi(
                        getValue(row, "IdCalle")
                    );

                vehiculo.placa =
                    getValue(row, "Placa");

                vehiculo.velocidad =
                    std::stod(
                        getValue(row, "Velocidad")
                    );

                vehiculo.segundoLlegada =
                    std::stoi(
                        getValue(row, "SegundoLlegada")
                    );

                engine.agregarVehiculo(vehiculo);
            }

            // Mantener los registros consistentes
            // cuando se inicia una simulacion configurada.
            db.executePrepared(
                "UPDATE SimulacionVehiculos SET "
                "Estado = 'Programado', "
                "HoraInicioCruce = NULL, "
                "HoraSalida = NULL, "
                "SegundoInicioCruce = NULL, "
                "SegundoSalida = NULL, "
                "TiempoEsperaSegundos = 0 "
                "WHERE IdSimulacion = ?",
                {std::to_string(idSimulacion)}
            );

            db.executePrepared(
                "UPDATE Simulaciones SET "
                "Estado = 'Ejecutando', "
                "FechaInicio = NOW(), "
                "FechaFin = NULL, "
                "TotalVehiculosFinalizados = 0 "
                "WHERE IdSimulacion = ?",
                {std::to_string(idSimulacion)}
            );

            // Preparar el motor antes del COMMIT.
            activeSimulations.emplace(
                idSimulacion,
                std::move(engine)
            );

            try {
                if (!db.commitTransaction()) {
                    throw std::runtime_error(
                        "Error confirmando transaccion"
                    );
                }

                transactionStarted = false;

            } catch (...) {
                activeSimulations.erase(idSimulacion);
                throw;
            }

            crow::json::wvalue response = summaryToJson(
                idSimulacion,
                activeSimulations.at(idSimulacion)
            );
            response["message"] = "Simulacion iniciada";

            return crow::response(200, response);

        } catch (const std::exception& error) {
            if (transactionStarted) {
                db.rollbackTransaction();
            }

            std::cerr
                << "Error iniciando simulacion: "
                << error.what() << std::endl;

            return errorResponse(
                500, "Error al iniciar la simulacion"
            );
        }
    });

    // =====================================================
    // FASE 5: AVANZAR UN SEGUNDO
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>/avanzar")
    .methods(crow::HTTPMethod::POST)
    ([](int idSimulacion) {
        return advanceSimulation(
            idSimulacion, 1
        );
    });

    // =====================================================
    // FASE 5: AVANZAR MULTIPLES SEGUNDOS
    // =====================================================

    CROW_ROUTE(
        app,
        "/api/simulaciones/<int>/avanzar-multiples"
    )
    .methods(crow::HTTPMethod::POST)
    ([](const crow::request& request, int idSimulacion) {
        const auto body =
            crow::json::load(request.body);

        if (!body ||
            body.t() != crow::json::type::Object) {
            return errorResponse(
                400,
                "Debe enviar un objeto JSON valido"
            );
        }

        int segundos = 0;

        if (!readInteger(
                body, "segundos", segundos, 1, 3600
            )) {
            return errorResponse(
                400,
                "El campo segundos debe ser un entero "
                "entre 1 y 3600"
            );
        }

        return advanceSimulation(
            idSimulacion, segundos
        );
    });

    // =====================================================
    // FASE 10: ESTADISTICAS DE SIMULACION
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>/estadisticas")
    .methods(crow::HTTPMethod::GET)
    ([](int idSimulacion) {
        if (idSimulacion <= 0) {
            return errorResponse(
                400, "Identificador de simulacion invalido"
            );
        }

        DatabaseConnection db;

        if (!connectDatabase(db)) {
            return errorResponse(
                503, "No se pudo conectar con MySQL"
            );
        }

        try {
            const auto simulaciones = db.selectPrepared(
                "SELECT IdSimulacion FROM Simulaciones "
                "WHERE IdSimulacion = ?",
                {std::to_string(idSimulacion)}
            );

            if (simulaciones.empty()) {
                return errorResponse(
                    404, "La simulacion no existe"
                );
            }

            crow::json::wvalue response =
                StatisticsService::obtenerEstadisticas(
                    db, idSimulacion
                );

            return crow::response(200, response);

        } catch (const std::exception& error) {
            std::cerr
                << "Error consultando estadisticas: "
                << error.what() << std::endl;

            return errorResponse(
                500, "Error al consultar estadisticas de la simulacion"
            );
        }
    });

    // =====================================================
    // FASE 5: CONSULTAR ESTADO EN MEMORIA
    // =====================================================

    CROW_ROUTE(app, "/api/simulaciones/<int>/estado")
    .methods(crow::HTTPMethod::GET)
    ([](int idSimulacion) {
        if (idSimulacion <= 0) {
            return errorResponse(
                400, "Identificador invalido"
            );
        }

        std::lock_guard<std::mutex> lock(
            simulationMutex
        );

        const auto it =
            activeSimulations.find(idSimulacion);

        if (it == activeSimulations.end()) {
            return errorResponse(
                404,
                "No existe una simulacion activa en memoria"
            );
        }

        crow::json::wvalue response =
            summaryToJson(
                idSimulacion,
                it->second
            );

        return crow::response(200, response);
    });
}