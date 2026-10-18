#include "traffic/SpeedControlService.h"

#include <stdexcept>
#include <string>
#include <vector>

int SpeedControlService::registrarMultas(
    DatabaseConnection& db,
    int idSimulacion,
    int segundoAnterior,
    int segundoActual
) {
    if (idSimulacion <= 0 ||
        segundoAnterior < 0 ||
        segundoActual < segundoAnterior) {
        throw std::invalid_argument(
            "Parametros invalidos para registrar multas."
        );
    }

    if (!db.isConnected()) {
        throw std::runtime_error(
            "No existe una conexion activa con MySQL."
        );
    }

    const std::string consulta =
        "INSERT INTO Multas ("
        "    IdSimulacionVehiculo, "
        "    IdCamara, "
        "    IdTarifaMulta, "
        "    VelocidadRegistrada, "
        "    VelocidadLimite, "
        "    MontoMulta, "
        "    SegundoDeteccion, "
        "    Estado"
        ") "
        "SELECT "
        "    sv.IdSimulacionVehiculo, "
        "    c.IdCamara, "
        "    t.IdTarifaMulta, "
        "    sv.Velocidad, "
        "    c.VelocidadLimite, "
        "    t.MontoMulta, "
        "    sv.SegundoLlegada, "
        "    'Generada' "
        "FROM SimulacionVehiculos sv "
        "INNER JOIN Camaras c "
        "    ON c.IdCalle = sv.IdCalle "
        "    AND c.EstadoRegistro = 'A' "
        "INNER JOIN TarifasMulta t "
        "    ON t.EstadoRegistro = 'A' "
        "    AND sv.Velocidad > t.VelocidadMinimaExclusiva "
        "    AND ("
        "        t.VelocidadMaximaInclusiva IS NULL "
        "        OR sv.Velocidad <= t.VelocidadMaximaInclusiva"
        "    ) "
        "WHERE sv.IdSimulacion = ? "
        "  AND sv.SegundoLlegada >= ? "
        "  AND sv.SegundoLlegada < ? "
        "  AND sv.Velocidad > c.VelocidadLimite "
        "  AND NOT EXISTS ("
        "      SELECT 1 "
        "      FROM Multas m "
        "      WHERE m.IdSimulacionVehiculo = sv.IdSimulacionVehiculo "
        "        AND m.IdCamara = c.IdCamara"
        "  ) "
        "ON DUPLICATE KEY UPDATE "
        "    IdMulta = IdMulta";

    const std::vector<std::string> parametros = {
        std::to_string(idSimulacion),
        std::to_string(segundoAnterior),
        std::to_string(segundoActual)
    };

    return static_cast<int>(
        db.executePrepared(consulta, parametros)
    );
}
