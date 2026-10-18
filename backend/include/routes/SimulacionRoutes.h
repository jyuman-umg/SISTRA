#ifndef SISTRA_SIMULACION_ROUTES_H
#define SISTRA_SIMULACION_ROUTES_H

#include <crow.h>
#include <crow/middlewares/cors.h>

void registerSimulacionRoutes(crow::App<crow::CORSHandler>& app);

#endif