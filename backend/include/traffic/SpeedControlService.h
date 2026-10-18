#ifndef SISTRA_SPEED_CONTROL_SERVICE_H
#define SISTRA_SPEED_CONTROL_SERVICE_H

#include "database/DatabaseConnection.h"

class SpeedControlService {
public:
    static int registrarMultas(
        DatabaseConnection& db,
        int idSimulacion,
        int segundoAnterior,
        int segundoActual
    );
};

#endif
