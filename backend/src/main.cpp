#include <crow.h>
#include <crow/middlewares/cors.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <system_error>

#include "routes/ApiRoutes.h"
#include "routes/VehiculoRoutes.h"
#include "routes/SimulacionRoutes.h"

namespace fs = std::filesystem;

// =====================================================
// LOCALIZAR LA CARPETA DEL FRONTEND
// =====================================================

fs::path obtenerDirectorioFrontend() {

    const fs::path directorioActual = fs::current_path();

    const std::vector<fs::path> rutas = {
        directorioActual / "frontend",
        directorioActual / ".." / "frontend",
        directorioActual / ".." / ".." / "frontend",
        directorioActual / ".." / ".." / ".." / "frontend"
    };

    for (const auto& ruta : rutas) {

        std::error_code error;

        const fs::path rutaNormalizada =
            fs::weakly_canonical(ruta, error);

        if (error) {
            continue;
        }

        if (fs::is_regular_file(rutaNormalizada / "index.html")) {
            return rutaNormalizada;
        }
    }

    return {};
}

// =====================================================
// OBTENER TIPO DE CONTENIDO
// =====================================================

std::string obtenerTipoContenido(const fs::path& archivo) {

    const std::string extension = archivo.extension().string();

    if (extension == ".html") {
        return "text/html; charset=utf-8";
    }

    if (extension == ".css") {
        return "text/css; charset=utf-8";
    }

    if (extension == ".js") {
        return "application/javascript; charset=utf-8";
    }

    if (extension == ".json") {
        return "application/json; charset=utf-8";
    }

    if (extension == ".png") {
        return "image/png";
    }

    if (extension == ".jpg" || extension == ".jpeg") {
        return "image/jpeg";
    }

    if (extension == ".svg") {
        return "image/svg+xml";
    }

    if (extension == ".ico") {
        return "image/x-icon";
    }

    if (extension == ".webp") {
        return "image/webp";
    }

    if (extension == ".woff") {
        return "font/woff";
    }

    if (extension == ".woff2") {
        return "font/woff2";
    }

    return "application/octet-stream";
}

// =====================================================
// LEER Y DEVOLVER UN ARCHIVO
// =====================================================

crow::response servirArchivo(const fs::path& archivo) {

    if (!fs::is_regular_file(archivo)) {
        return crow::response(404, "Archivo no encontrado");
    }

    std::ifstream entrada(archivo, std::ios::binary);

    if (!entrada.is_open()) {
        return crow::response(500, "No se pudo abrir el archivo");
    }

    std::ostringstream contenido;
    contenido << entrada.rdbuf();

    if (entrada.bad()) {
        return crow::response(500, "Error al leer el archivo");
    }

    crow::response respuesta(200, contenido.str());

    respuesta.set_header(
        "Content-Type",
        obtenerTipoContenido(archivo)
    );

    return respuesta;
}

// =====================================================
// VALIDAR Y SERVIR ARCHIVOS DEL FRONTEND
// =====================================================

crow::response servirRecurso(
    const fs::path& directorioFrontend,
    const std::string& carpeta,
    const std::string& nombreArchivo
) {

    const fs::path directorioBase =
        (directorioFrontend / carpeta).lexically_normal();

    const fs::path rutaArchivo =
        (directorioBase / fs::path(nombreArchivo)).lexically_normal();

    // Impedir que una solicitud acceda fuera de la carpeta.
    const fs::path rutaRelativa =
        rutaArchivo.lexically_relative(directorioBase);

    if (rutaRelativa.empty() || rutaRelativa.is_absolute()) {
        return crow::response(403, "Acceso denegado");
    }

    for (const auto& parte : rutaRelativa) {

        if (parte == "..") {
            return crow::response(403, "Acceso denegado");
        }
    }

    // Evitar que enlaces simbolicos permitan salir de la carpeta.
    std::error_code errorBase;
    std::error_code errorArchivo;

    const fs::path baseCanonica =
        fs::weakly_canonical(directorioBase, errorBase);

    const fs::path archivoCanonico =
        fs::weakly_canonical(rutaArchivo, errorArchivo);

    if (errorBase || errorArchivo) {
        return crow::response(404, "Archivo no encontrado");
    }

    const fs::path relativaCanonica =
        archivoCanonico.lexically_relative(baseCanonica);

    if (relativaCanonica.empty() || relativaCanonica.is_absolute()) {
        return crow::response(403, "Acceso denegado");
    }

    for (const auto& parte : relativaCanonica) {

        if (parte == "..") {
            return crow::response(403, "Acceso denegado");
        }
    }

    return servirArchivo(archivoCanonico);
}

// =====================================================
// INICIO DEL SISTEMA
// =====================================================

int main() {

    // =====================================================
    // CONFIGURACION DEL SERVIDOR
    // =====================================================

    crow::App<crow::CORSHandler> app;

    // =====================================================
    // CONFIGURACION CORS
    // =====================================================

    auto& cors = app.get_middleware<crow::CORSHandler>();

    cors.global()
        .origin("http://localhost:18080")
        .methods(
            crow::HTTPMethod::GET,
            crow::HTTPMethod::POST,
            crow::HTTPMethod::OPTIONS
        )
        .headers("Content-Type");

    // =====================================================
    // ENCABEZADO DEL SISTEMA
    // =====================================================

    std::cout << "====================================="
              << std::endl;

    std::cout << " SISTRA - SISTEMA DE CONTROL DE TRAFICO"
              << std::endl;

    std::cout << "====================================="
              << std::endl;

    // =====================================================
    // LOCALIZAR FRONTEND
    // =====================================================

    const fs::path directorioFrontend =
        obtenerDirectorioFrontend();

    if (directorioFrontend.empty()) {

        std::cerr << std::endl;

        std::cerr
            << "ERROR: No se encontro frontend/index.html"
            << std::endl;

        std::cerr
            << "Verifique la estructura del proyecto SISTRA."
            << std::endl;

        return 1;
    }

    std::cout << std::endl;

    std::cout
        << "Frontend encontrado en: "
        << directorioFrontend.string()
        << std::endl;

    // =====================================================
    // REGISTRO DE RUTAS DE LA API REST
    // =====================================================

    registerApiRoutes(app);
    registerVehiculoRoutes(app);
    registerSimulacionRoutes(app);

    // =====================================================
    // RUTA PRINCIPAL DEL FRONTEND
    // =====================================================

    CROW_ROUTE(app, "/")
    ([directorioFrontend]() {

        return servirArchivo(
            directorioFrontend / "index.html"
        );
    });

    // =====================================================
    // ARCHIVOS CSS
    // =====================================================

    CROW_ROUTE(app, "/css/<path>")
    ([directorioFrontend](const std::string& archivo) {

        return servirRecurso(
            directorioFrontend,
            "css",
            archivo
        );
    });

    // =====================================================
    // ARCHIVOS JAVASCRIPT
    // =====================================================

    CROW_ROUTE(app, "/js/<path>")
    ([directorioFrontend](const std::string& archivo) {

        return servirRecurso(
            directorioFrontend,
            "js",
            archivo
        );
    });

    // =====================================================
    // RECURSOS GRAFICOS
    // =====================================================

    CROW_ROUTE(app, "/assets/<path>")
    ([directorioFrontend](const std::string& archivo) {

        return servirRecurso(
            directorioFrontend,
            "assets",
            archivo
        );
    });

    // =====================================================
    // INICIAR SERVIDOR
    // =====================================================

    std::cout << std::endl;

    std::cout << "Servidor SISTRA iniciado."
              << std::endl;

    std::cout
        << "Backend:  http://localhost:18080/api/health"
        << std::endl;

    std::cout
        << "Frontend: http://localhost:18080/"
        << std::endl;

    std::cout << std::endl;

    app.port(18080).multithreaded().run();

    return 0;
}