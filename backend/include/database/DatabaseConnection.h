#ifndef SISTRA_DATABASE_CONNECTION_H
#define SISTRA_DATABASE_CONNECTION_H

#include <mysql.h>

#include <string>
#include <vector>
#include <map>

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

    void disconnect();

    bool isConnected() const;
};

#endif