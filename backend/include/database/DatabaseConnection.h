#ifndef SISTRA_DATABASE_CONNECTION_H
#define SISTRA_DATABASE_CONNECTION_H

#include <mysql.h>

#include <map>
#include <string>
#include <vector>

class DatabaseConnection {
private:
    MYSQL* connection;

public:
    DatabaseConnection();
    ~DatabaseConnection();

    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    bool connect(
        const std::string& host,
        const std::string& user,
        const std::string& password,
        const std::string& database,
        unsigned int port = 3306
    );

    bool executeQuery(const std::string& query);

    std::vector<std::map<std::string, std::string>> select(
        const std::string& query
    );

    // Ejecuta una consulta INSERT con parametros seguros.
    unsigned long long insert(
        const std::string& query,
        const std::vector<std::string>& parameters
    );

    // Ejecuta UPDATE o DELETE utilizando parametros seguros.
    unsigned long long executePrepared(
        const std::string& query,
        const std::vector<std::string>& parameters
    );

    // Consulta registros utilizando parametros seguros.
    std::vector<std::map<std::string, std::string>> selectPrepared(
        const std::string& query,
        const std::vector<std::string>& parameters
    );


    // Inicia una transaccion en MySQL.
    bool beginTransaction();

    // Confirma los cambios de la transaccion.
    bool commitTransaction();

    // Revierte los cambios de la transaccion.
    bool rollbackTransaction();

    void disconnect();

    bool isConnected() const;
};

#endif
