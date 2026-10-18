#ifndef SISTRA_VEHICULO_ROUTES_H
#define SISTRA_VEHICULO_ROUTES_H

#include <crow.h>
#include <crow/middlewares/cors.h>

void registerVehiculoRoutes(crow::App<crow::CORSHandler>& app);

#endif
