#ifndef CONFIG_H
#define CONFIG_H

#include "database.h"
#include <cstdlib>

inline DbConfig getDbConfig()
{
    DbConfig config;

    const char *host = std::getenv("DB_HOST");
    const char *port = std::getenv("DB_PORT");
    const char *user = std::getenv("DB_USER");
    const char *password = std::getenv("DB_PASSWORD");
    const char *database = std::getenv("DB_NAME");

    config.host = host ? host : "localhost";
    config.port = port ? static_cast<unsigned int>(std::atoi(port)) : 3306;
    config.user = user ? user : "root";
    config.password = password ? password : "";
    config.database = database ? database : "cpu_scheduling_db";

    return config;
}

#endif