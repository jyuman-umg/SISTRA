#include "statistics/StatisticsService.h"

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

    std::string obtenerValor(
        const std::map<std::string, std::string>& fila,
        const std::string& columna
    ) {
        const auto it = fila.find(columna);

        if (it == fila.end()) {
            throw std::runtime_error(
                "Columna no encontrada: " + columna
            );
        }

        return it->second;
    }

    int obtenerEntero(
        const std::map<std::string, std::string>& fila,
        const std::string& columna
    ) {
        const std::string valor = obtenerValor(fila, columna);

        return valor.empty() ? 0 : std::stoi(valor);
    }

    double obtenerDecimal(
        const std::map<std::string, std::string>& fila,
        const std::string& columna
    ) {
        const std::string valor = obtenerValor(fila, columna);

        return valor.empty() ? 0.0 : std::stod(valor);
    }

}

crow::json::wvalue StatisticsService::obtenerEstadisticas(
    DatabaseConnection& db,
    int idSimulacion
) {
    if (idSimulacion <= 0) {
        throw std::invalid_argument(
            "El identificador de simulacion es invalido."
        );
    }

    if (!db.isConnected()) {
        throw std::runtime_error(
            "No existe una conexion activa con MySQL."
        );
    }

    const std::string id = std::to_string(idSimulacion);

    // =====================================================
    // INFORMACION GENERAL DE LA SIMULACION
    // =====================================================

    const auto simulaciones = db.selectPrepared(
        "SELECT "
        "IdSimulacion, "
        "Nombre, "
        "Estado, "
        "DuracionSimuladaSegundos "
        "FROM Simulaciones "
        "WHERE IdSimulacion = ?",
        {id}
    );

    if (simulaciones.empty()) {
        throw std::invalid_argument(
            "La simulacion no existe."
        );
    }

    const auto& simulacion = simulaciones.front();

    // =====================================================
    // ESTADISTICAS GENERALES DE VEHICULOS
    // =====================================================

    const auto resumenVehiculos = db.selectPrepared(
        "SELECT "
        "COUNT(*) AS TotalVehiculos, "
        "COALESCE(SUM(CASE WHEN Estado = 'Programado' "
        "THEN 1 ELSE 0 END), 0) AS Programados, "
        "COALESCE(SUM(CASE WHEN Estado = 'EnCola' "
        "THEN 1 ELSE 0 END), 0) AS EnCola, "
        "COALESCE(SUM(CASE WHEN Estado = 'Cruzando' "
        "THEN 1 ELSE 0 END), 0) AS Cruzando, "
        "COALESCE(SUM(CASE WHEN Estado = 'Finalizado' "
        "THEN 1 ELSE 0 END), 0) AS Finalizados, "
        "COALESCE(AVG(CASE "
        "WHEN Estado = 'Finalizado' "
        "THEN TiempoEsperaSegundos "
        "ELSE NULL END), 0) AS TiempoPromedioEspera "
        "FROM SimulacionVehiculos "
        "WHERE IdSimulacion = ?",
        {id}
    );

    const auto& vehiculos = resumenVehiculos.front();

    // =====================================================
    // ESTADISTICAS POR CALLE
    // =====================================================

    const auto resumenCalles = db.selectPrepared(
        "SELECT "
        "c.IdCalle, "
        "c.Nombre, "
        "COUNT(sv.IdSimulacionVehiculo) AS TotalVehiculos, "
        "COALESCE(SUM(CASE "
        "WHEN sv.Estado = 'Finalizado' "
        "THEN 1 ELSE 0 END), 0) AS Finalizados, "
        "COALESCE(SUM(CASE "
        "WHEN sv.Estado = 'EnCola' "
        "THEN 1 ELSE 0 END), 0) AS EnCola, "
        "COALESCE(AVG(CASE "
        "WHEN sv.Estado = 'Finalizado' "
        "THEN sv.TiempoEsperaSegundos "
        "ELSE NULL END), 0) AS TiempoPromedioEspera "
        "FROM Calles c "
        "LEFT JOIN SimulacionVehiculos sv "
        "ON sv.IdCalle = c.IdCalle "
        "AND sv.IdSimulacion = ? "
        "GROUP BY c.IdCalle, c.Nombre "
        "ORDER BY c.IdCalle",
        {id}
    );

    // =====================================================
    // ESTADISTICAS DE MULTAS
    // =====================================================

    const auto resumenMultas = db.selectPrepared(
        "SELECT "
        "COUNT(*) AS TotalMultas, "
        "COALESCE(SUM(m.MontoMulta), 0) AS MontoTotal "
        "FROM Multas m "
        "INNER JOIN SimulacionVehiculos sv "
        "ON sv.IdSimulacionVehiculo = m.IdSimulacionVehiculo "
        "WHERE sv.IdSimulacion = ? "
        "AND m.Estado = 'Generada'",
        {id}
    );

    const auto& multas = resumenMultas.front();

    // =====================================================
    // CONSTRUCCION DE RESPUESTA JSON
    // =====================================================

    crow::json::wvalue response;

    response["success"] = true;
    response["idSimulacion"] = idSimulacion;
    response["nombreSimulacion"] =
        obtenerValor(simulacion, "Nombre");
    response["estadoSimulacion"] =
        obtenerValor(simulacion, "Estado");

    response["duracionSegundos"] =
        obtenerEntero(
            simulacion,
            "DuracionSimuladaSegundos"
        );

    crow::json::wvalue estadisticas;

    estadisticas["totalVehiculos"] =
        obtenerEntero(vehiculos, "TotalVehiculos");

    estadisticas["programados"] =
        obtenerEntero(vehiculos, "Programados");

    estadisticas["enCola"] =
        obtenerEntero(vehiculos, "EnCola");

    estadisticas["cruzando"] =
        obtenerEntero(vehiculos, "Cruzando");

    estadisticas["finalizados"] =
        obtenerEntero(vehiculos, "Finalizados");

    estadisticas["tiempoPromedioEsperaSegundos"] =
        obtenerDecimal(
            vehiculos,
            "TiempoPromedioEspera"
        );

    // =====================================================
    // LISTADO DE ESTADISTICAS POR CALLE
    // =====================================================

    crow::json::wvalue::list calles;

    for (const auto& fila : resumenCalles) {
        crow::json::wvalue calle;

        calle["idCalle"] =
            obtenerEntero(fila, "IdCalle");

        calle["nombre"] =
            obtenerValor(fila, "Nombre");

        calle["totalVehiculos"] =
            obtenerEntero(fila, "TotalVehiculos");

        calle["finalizados"] =
            obtenerEntero(fila, "Finalizados");

        calle["enCola"] =
            obtenerEntero(fila, "EnCola");

        calle["tiempoPromedioEsperaSegundos"] =
            obtenerDecimal(
                fila,
                "TiempoPromedioEspera"
            );

        calles.push_back(std::move(calle));
    }

    estadisticas["calles"] = std::move(calles);

    // =====================================================
    // RESUMEN DE MULTAS
    // =====================================================

    crow::json::wvalue estadisticasMultas;

    estadisticasMultas["totalMultas"] =
        obtenerEntero(multas, "TotalMultas");

    estadisticasMultas["montoTotal"] =
        obtenerDecimal(multas, "MontoTotal");

    estadisticas["multas"] =
        std::move(estadisticasMultas);

    response["estadisticas"] =
        std::move(estadisticas);

    return response;
}
