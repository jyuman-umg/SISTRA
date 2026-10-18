#ifndef SISTRA_STATISTICS_SERVICE_H
#define SISTRA_STATISTICS_SERVICE_H

#include "database/DatabaseConnection.h"

#include <crow.h>

class StatisticsService {
public:
    static crow::json::wvalue obtenerEstadisticas(
        DatabaseConnection& db,
        int idSimulacion
    );
};

#endif