#include "database/DatabaseConnection.h"
#include <cstdlib>
#include <iostream>

DatabaseConnection::DatabaseConnection()
    : connection(nullptr) {
}

DatabaseConnection::~DatabaseConnection() {
    disconnect();
}

bool DatabaseConnection::connect(
    const std::string& host,
    const std::string& user,
    const std::string& password,
    const std::string& database,
    unsigned int port
) {
    disconnect();

    connection = mysql_init(nullptr);

    if (connection == nullptr) {
        std::cerr << "Error al inicializar MySQL." << std::endl;
        return false;
    }


    // Configurar la carpeta de plugins de autenticacion.
    const char* pluginDirectory =
        std::getenv("SISTRA_DB_PLUGIN_DIR");

    if (pluginDirectory != nullptr &&
        pluginDirectory[0] != '\0') {

        if (mysql_options(
                connection,
                MYSQL_PLUGIN_DIR,
                pluginDirectory
            ) != 0) {

            std::cerr << "Error al configurar plugins MySQL: "
                      << mysql_error(connection)
                      << std::endl;

            disconnect();
            return false;
            }
        }


    if (mysql_real_connect(
            connection,
            host.c_str(),
            user.c_str(),
            password.c_str(),
            database.c_str(),
            port,
            nullptr,
            0
        ) == nullptr) {

        std::cerr << "Error de conexion MySQL: "
                  << mysql_error(connection)
                  << std::endl;

        disconnect();
        return false;
    }

    std::cout << "Conexion a MySQL establecida correctamente."
              << std::endl;

    return true;
}

bool DatabaseConnection::executeQuery(
    const std::string& query
) {
    if (!isConnected()) {
        std::cerr << "No existe una conexion activa."
                  << std::endl;
        return false;
    }

    if (mysql_query(connection, query.c_str()) != 0) {
        std::cerr << "Error al ejecutar consulta: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result != nullptr) {
        unsigned int columns = mysql_num_fields(result);
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)) != nullptr) {
            for (unsigned int i = 0; i < columns; i++) {
                std::cout << (row[i] != nullptr ? row[i] : "NULL");

                if (i + 1 < columns) {
                    std::cout << " | ";
                }
            }

            std::cout << std::endl;
        }

        mysql_free_result(result);
    } else if (mysql_field_count(connection) != 0) {
        std::cerr << "Error al recuperar resultados: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    return true;
}

void DatabaseConnection::disconnect() {
    if (connection != nullptr) {
        mysql_close(connection);
        connection = nullptr;
    }
}

bool DatabaseConnection::isConnected() const {
    return connection != nullptr;
}
