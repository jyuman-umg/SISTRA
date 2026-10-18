#ifndef SISTRA_API_ROUTES_H
#define SISTRA_API_ROUTES_H

#include <crow.h>
#include <crow/middlewares/cors.h>

void registerApiRoutes(crow::App<crow::CORSHandler>& app);

#endif
