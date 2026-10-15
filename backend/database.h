// database.h - all MySQL access of the project (connection, queries, transactions)
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include "models/SchedulingResult.h"

struct DbConfig
{
    std::string host;
    unsigned int port = 3306;
    std::string user;
    std::string password;
    std::string database;
};

enum DbStatus
{
    DB_OK,
    DB_NOT_FOUND,
    DB_ERROR
};

class Database
{
public:
    explicit Database(const DbConfig &config);
    ~Database();

    bool connect(std::string &error);

    // ---- workloads ----
    // Saves a workload and its processes in ONE transaction.
    bool saveWorkload(const std::string &name, const std::string &description,
                      const std::vector<Process> &processes, long long &workloadId, std::string &error);
    bool listWorkloads(std::string &json, std::string &error);                     // JSON array
    DbStatus getWorkloadJson(long long id, std::string &json, std::string &error); // {"workload":{},"processes":[]}
    DbStatus loadWorkload(long long id, std::string &name, std::vector<Process> &processes, std::string &error);

    // ---- scheduling runs ----
    // Saves run + process results + Gantt segments in ONE transaction (COMMIT / ROLLBACK).
    bool saveRun(long long workloadId, const SchedulingResult &result, long long &runId, std::string &error);
    bool listRuns(std::string &json, std::string &error);                            // JSON array
    DbStatus getRunJson(long long runId, std::string &json, std::string &error);     // {"run":{},"results":[],"gantt":[]}
    bool compareStored(long long workloadId, std::string &json, std::string &error); // CALL sp_compare_algorithms
    bool getStats(std::string &json, std::string &error);                            // dashboard numbers

private:
    DbConfig config;
    void *handle; // the MYSQL* connection (kept as void* so this header does not need mysql.h)

    bool ensureConnected(std::string &error);
    bool execute(const std::string &sql, std::string &error);
    bool queryRows(const std::string &sql, std::vector<std::string> &rows, std::string &error); // each row = JSON object
    bool queryTable(const std::string &sql, std::vector<std::vector<std::string>> &table, std::string &error);
    std::string escape(const std::string &text);
    void rollbackQuietly();
};

#endif
