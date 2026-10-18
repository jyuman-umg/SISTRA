#include "database/DatabaseConnection.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <utility>

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


std::vector<std::map<std::string, std::string>>
DatabaseConnection::select(const std::string& query) {

    std::vector<std::map<std::string, std::string>> records;

    if (!isConnected()) {
        throw std::runtime_error(
            "No existe una conexion activa con MySQL."
        );
    }

    if (mysql_query(connection, query.c_str()) != 0) {
        throw std::runtime_error(
            std::string("Error al ejecutar consulta: ") +
            mysql_error(connection)
        );
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        throw std::runtime_error(
            std::string("Error al obtener resultados: ") +
            mysql_error(connection)
        );
    }

    const unsigned int columnCount = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != nullptr) {
        std::map<std::string, std::string> record;

        for (unsigned int i = 0; i < columnCount; i++) {
            record[fields[i].name] =
                row[i] != nullptr ? row[i] : "";
        }

        records.push_back(std::move(record));
    }

    mysql_free_result(result);

    return records;
}

unsigned long long DatabaseConnection::insert(
    const std::string& query,
    const std::vector<std::string>& parameters
) {
    if (!isConnected()) {
        throw std::runtime_error(
            "No existe una conexion activa con MySQL."
        );
    }

    MYSQL_STMT* statement = mysql_stmt_init(connection);

    if (statement == nullptr) {
        throw std::runtime_error(
            "No se pudo inicializar la consulta preparada."
        );
    }

    std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>
        statementGuard(statement, &mysql_stmt_close);

    if (mysql_stmt_prepare(
            statement,
            query.c_str(),
            static_cast<unsigned long>(query.size())
        ) != 0) {

        throw std::runtime_error(
            std::string("Error preparando INSERT: ") +
            mysql_stmt_error(statement)
        );
    }

    if (mysql_stmt_param_count(statement) != parameters.size()) {
        throw std::runtime_error(
            "La cantidad de parametros no coincide con la consulta."
        );
    }

    std::vector<MYSQL_BIND> bindings(parameters.size());
    std::vector<unsigned long> lengths(parameters.size());

    for (std::size_t i = 0; i < parameters.size(); i++) {
        MYSQL_BIND binding{};

        lengths[i] =
            static_cast<unsigned long>(parameters[i].size());

        binding.buffer_type = MYSQL_TYPE_STRING;
        binding.buffer =
            const_cast<char*>(parameters[i].data());
        binding.buffer_length = lengths[i];
        binding.length = &lengths[i];

        bindings[i] = binding;
    }

    if (!bindings.empty() &&
        mysql_stmt_bind_param(statement, bindings.data()) != 0) {

        throw std::runtime_error(
            std::string("Error vinculando parametros: ") +
            mysql_stmt_error(statement)
        );
    }

    if (mysql_stmt_execute(statement) != 0) {
        throw std::runtime_error(
            std::string("Error ejecutando INSERT: ") +
            mysql_stmt_error(statement)
        );
    }

    return static_cast<unsigned long long>(
        mysql_stmt_insert_id(statement)
    );
}


std::vector<std::map<std::string, std::string>>
DatabaseConnection::selectPrepared(
    const std::string& query,
    const std::vector<std::string>& parameters
) {
    if (!isConnected()) {
        throw std::runtime_error(
            "No existe una conexion activa con MySQL."
        );
    }

    MYSQL_STMT* statement = mysql_stmt_init(connection);

    if (statement == nullptr) {
        throw std::runtime_error(
            "No se pudo inicializar la consulta preparada."
        );
    }

    std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>
        statementGuard(statement, &mysql_stmt_close);

    if (mysql_stmt_prepare(
            statement,
            query.c_str(),
            static_cast<unsigned long>(query.size())
        ) != 0) {

        throw std::runtime_error(
            std::string("Error preparando SELECT: ") +
            mysql_stmt_error(statement)
        );
    }

    if (mysql_stmt_param_count(statement) != parameters.size()) {
        throw std::runtime_error(
            "La cantidad de parametros no coincide con la consulta."
        );
    }

    std::vector<MYSQL_BIND> parameterBindings(parameters.size());
    std::vector<unsigned long> parameterLengths(parameters.size());

    for (std::size_t i = 0; i < parameters.size(); i++) {
        MYSQL_BIND binding{};

        parameterLengths[i] =
            static_cast<unsigned long>(parameters[i].size());

        binding.buffer_type = MYSQL_TYPE_STRING;
        binding.buffer =
            const_cast<char*>(parameters[i].data());
        binding.buffer_length = parameterLengths[i];
        binding.length = &parameterLengths[i];

        parameterBindings[i] = binding;
    }

    if (!parameterBindings.empty() &&
        mysql_stmt_bind_param(
            statement,
            parameterBindings.data()
        ) != 0) {

        throw std::runtime_error(
            std::string("Error vinculando parametros: ") +
            mysql_stmt_error(statement)
        );
    }

    if (mysql_stmt_execute(statement) != 0) {
        throw std::runtime_error(
            std::string("Error ejecutando SELECT: ") +
            mysql_stmt_error(statement)
        );
    }

    MYSQL_RES* metadata = mysql_stmt_result_metadata(statement);

    if (metadata == nullptr) {
        throw std::runtime_error(
            "La consulta no devolvio columnas."
        );
    }

    std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)>
        metadataGuard(metadata, &mysql_free_result);

    const unsigned int columnCount =
        mysql_num_fields(metadata);

    MYSQL_FIELD* fields = mysql_fetch_fields(metadata);

    std::vector<std::string> columnNames;
    columnNames.reserve(columnCount);

    for (unsigned int i = 0; i < columnCount; i++) {
        columnNames.emplace_back(fields[i].name);
    }

    if (mysql_stmt_store_result(statement) != 0) {
        throw std::runtime_error(
            std::string("Error almacenando resultados: ") +
            mysql_stmt_error(statement)
        );
    }

    std::vector<MYSQL_BIND> resultBindings(columnCount);
    std::vector<unsigned long> resultLengths(columnCount);
    std::vector<my_bool> nullFlags(columnCount);
    std::vector<std::vector<char>> buffers(
        columnCount,
        std::vector<char>(256)
    );

    for (unsigned int i = 0; i < columnCount; i++) {
        MYSQL_BIND binding{};

        binding.buffer_type = MYSQL_TYPE_STRING;
        binding.buffer = buffers[i].data();
        binding.buffer_length =
            static_cast<unsigned long>(buffers[i].size());
        binding.length = &resultLengths[i];
        binding.is_null = &nullFlags[i];

        resultBindings[i] = binding;
    }

    if (mysql_stmt_bind_result(
            statement,
            resultBindings.data()
        ) != 0) {

        throw std::runtime_error(
            std::string("Error vinculando resultados: ") +
            mysql_stmt_error(statement)
        );
    }

    std::vector<std::map<std::string, std::string>> records;

    int fetchStatus;

    while ((fetchStatus = mysql_stmt_fetch(statement)) == 0 ||
           fetchStatus == MYSQL_DATA_TRUNCATED) {

        std::map<std::string, std::string> record;

        for (unsigned int i = 0; i < columnCount; i++) {
            if (nullFlags[i]) {
                record[columnNames[i]] = "";
                continue;
            }

            if (resultLengths[i] > buffers[i].size()) {
                buffers[i].resize(resultLengths[i]);

                MYSQL_BIND columnBinding{};
                columnBinding.buffer_type = MYSQL_TYPE_STRING;
                columnBinding.buffer = buffers[i].data();
                columnBinding.buffer_length =
                    static_cast<unsigned long>(buffers[i].size());
                columnBinding.length = &resultLengths[i];

                if (mysql_stmt_fetch_column(
                        statement,
                        &columnBinding,
                        i,
                        0
                    ) != 0) {

                    throw std::runtime_error(
                        "Error recuperando una columna."
                    );
                }
            }

            record[columnNames[i]] = std::string(
                buffers[i].data(),
                resultLengths[i]
            );
        }

        records.push_back(std::move(record));
    }

    if (fetchStatus != MYSQL_NO_DATA) {
        throw std::runtime_error(
            std::string("Error recuperando filas: ") +
            mysql_stmt_error(statement)
        );
    }

    return records;
}


bool DatabaseConnection::beginTransaction() {
    if (!isConnected()) {
        std::cerr << "No existe una conexion activa."
                  << std::endl;
        return false;
    }

    if (mysql_autocommit(connection, 0) != 0) {
        std::cerr << "Error iniciando transaccion: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    return true;
}

bool DatabaseConnection::commitTransaction() {
    if (!isConnected()) {
        return false;
    }

    if (mysql_commit(connection) != 0) {
        std::cerr << "Error confirmando transaccion: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    if (mysql_autocommit(connection, 1) != 0) {
        std::cerr << "Error restaurando autocommit: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    return true;
}

bool DatabaseConnection::rollbackTransaction() {
    if (!isConnected()) {
        return false;
    }

    const bool rollbackSuccess =
        mysql_rollback(connection) == 0;

    if (!rollbackSuccess) {
        std::cerr << "Error revirtiendo transaccion: "
                  << mysql_error(connection)
                  << std::endl;
    }

    if (mysql_autocommit(connection, 1) != 0) {
        std::cerr << "Error restaurando autocommit: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    return rollbackSuccess;
}


unsigned long long DatabaseConnection::executePrepared(
    const std::string& query,
    const std::vector<std::string>& parameters
) {
    if (!isConnected()) {
        throw std::runtime_error(
            "No existe una conexion activa con MySQL."
        );
    }

    MYSQL_STMT* statement = mysql_stmt_init(connection);

    if (statement == nullptr) {
        throw std::runtime_error(
            "No se pudo inicializar la consulta preparada."
        );
    }

    std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>
        statementGuard(statement, &mysql_stmt_close);

    if (mysql_stmt_prepare(
            statement,
            query.c_str(),
            static_cast<unsigned long>(query.size())
        ) != 0) {

        throw std::runtime_error(
            std::string("Error preparando consulta: ") +
            mysql_stmt_error(statement)
        );
    }

    if (mysql_stmt_param_count(statement) != parameters.size()) {
        throw std::runtime_error(
            "La cantidad de parametros no coincide con la consulta."
        );
    }

    std::vector<MYSQL_BIND> bindings(parameters.size());
    std::vector<unsigned long> lengths(parameters.size());

    for (std::size_t i = 0; i < parameters.size(); i++) {
        MYSQL_BIND binding{};

        lengths[i] =
            static_cast<unsigned long>(parameters[i].size());

        binding.buffer_type = MYSQL_TYPE_STRING;
        binding.buffer =
            const_cast<char*>(parameters[i].data());
        binding.buffer_length = lengths[i];
        binding.length = &lengths[i];

        bindings[i] = binding;
    }

    if (!bindings.empty() &&
        mysql_stmt_bind_param(statement, bindings.data()) != 0) {

        throw std::runtime_error(
            std::string("Error vinculando parametros: ") +
            mysql_stmt_error(statement)
        );
    }

    if (mysql_stmt_execute(statement) != 0) {
        throw std::runtime_error(
            std::string("Error ejecutando consulta: ") +
            mysql_stmt_error(statement)
        );
    }

    return static_cast<unsigned long long>(
        mysql_stmt_affected_rows(statement)
    );
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
