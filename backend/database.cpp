// database.cpp - MySQL access using the MySQL C client library (libmysql, the same library
// that MySQL Connector/C++ is built on). Every statement is plain, readable SQL.
#include "database.h"
#include "json.h"
#ifdef _WIN32
#include <winsock2.h> // must come before mysql.h on Windows
#endif
#include <mysql.h>
#include <cstdio>
#include <cstdlib>


static std::string num(double value, int decimals)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%.*f", decimals, value);
    return buf;
}

static std::string toStr(long long v) { return std::to_string(v); }

// true when a column should be written as a JSON number (not as a quoted string)
static bool isNumericType(enum_field_types t)
{
    switch (t)
    {
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
        return true;
    default:
        return false;
    }
}

static std::string joinRows(const std::vector<std::string> &rows)
{
    std::string out = "[";
    for (size_t i = 0; i < rows.size(); i++)
    {
        if (i)
            out += ",";
        out += rows[i];
    }
    return out + "]";
}

// ------------------------------------------------------------------
// connection
// ------------------------------------------------------------------
// the connection handle, cast back to the real MySQL type
#define conn (static_cast<MYSQL *>(handle))

Database::Database(const DbConfig &cfg) : config(cfg), handle(nullptr) {}

Database::~Database()
{
    if (handle)
        mysql_close(conn);
}

bool Database::connect(std::string &error)
{
    if (handle)
    {
        mysql_close(conn);
        handle = nullptr;
    }
    handle = mysql_init(nullptr);
    if (!handle)
    {
        error = "Database connection failed: could not initialise the MySQL client library.";
        return false;
    }
    unsigned int timeoutSeconds = 5;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeoutSeconds);
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    // CLIENT_MULTI_RESULTS is needed to read the result of a stored procedure (CALL)
    if (!mysql_real_connect(conn, config.host.c_str(), config.user.c_str(), config.password.c_str(),
                            config.database.c_str(), config.port, nullptr, CLIENT_MULTI_RESULTS))
    {
        error = std::string("Database connection failed: ") + mysql_error(conn) +
                ". Check that MySQL is running, that schema.sql was executed and that the user/password in config.h are correct.";
        mysql_close(conn);
        handle = nullptr;
        return false;
    }
    return true;
}

bool Database::ensureConnected(std::string &error)
{
    if (handle && mysql_ping(conn) == 0)
        return true;
    return connect(error);
}

// ------------------------------------------------------------------
// low level query helpers
// ------------------------------------------------------------------
std::string Database::escape(const std::string &text)
{
    std::string out(text.size() * 2 + 1, '\0');
    unsigned long n = mysql_real_escape_string(conn, &out[0], text.c_str(), (unsigned long)text.size());
    out.resize(n);
    return out;
}

bool Database::execute(const std::string &sql, std::string &error)
{
    if (mysql_query(conn, sql.c_str()) != 0)
    {
        error = std::string("Database error: ") + mysql_error(conn);
        return false;
    }
    return true;
}

void Database::rollbackQuietly()
{
    if (handle)
        mysql_query(conn, "ROLLBACK");
}

// Runs a query and gives every row back as a JSON object string.
bool Database::queryRows(const std::string &sql, std::vector<std::string> &rows, std::string &error)
{
    rows.clear();
    if (!ensureConnected(error))
        return false;
    if (mysql_query(conn, sql.c_str()) != 0)
    {
        error = std::string("Database error: ") + mysql_error(conn);
        return false;
    }
    MYSQL_RES *result = mysql_store_result(conn);
    // a CALL statement returns extra (empty) results - read and discard them
    while (mysql_next_result(conn) == 0)
    {
        MYSQL_RES *extra = mysql_store_result(conn);
        if (extra)
            mysql_free_result(extra);
    }
    if (!result)
    {
        error = std::string("Database error: ") + mysql_error(conn);
        return false;
    }

    unsigned int columns = mysql_num_fields(result);
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        unsigned long *lengths = mysql_fetch_lengths(result);
        std::string obj = "{";
        for (unsigned int c = 0; c < columns; c++)
        {
            if (c)
                obj += ",";
            obj += jsonString(fields[c].name) + ":";
            if (row[c] == nullptr)
                obj += "null";
            else if (isNumericType(fields[c].type))
                obj += row[c];
            else
                obj += jsonString(std::string(row[c], lengths[c]));
        }
        obj += "}";
        rows.push_back(obj);
    }
    mysql_free_result(result);
    return true;
}

// Runs a query and gives the raw text values (used when C++ needs the numbers).
bool Database::queryTable(const std::string &sql, std::vector<std::vector<std::string>> &table, std::string &error)
{
    table.clear();
    if (!ensureConnected(error))
        return false;
    if (mysql_query(conn, sql.c_str()) != 0)
    {
        error = std::string("Database error: ") + mysql_error(conn);
        return false;
    }
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result)
    {
        error = std::string("Database error: ") + mysql_error(conn);
        return false;
    }
    unsigned int columns = mysql_num_fields(result);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        std::vector<std::string> line;
        for (unsigned int c = 0; c < columns; c++)
            line.push_back(row[c] ? row[c] : "");
        table.push_back(line);
    }
    mysql_free_result(result);
    return true;
}

// ------------------------------------------------------------------
// workloads
// ------------------------------------------------------------------
bool Database::saveWorkload(const std::string &name, const std::string &description,
                            const std::vector<Process> &processes, long long &workloadId, std::string &error)
{
    if (!ensureConnected(error))
        return false;

    if (!execute("START TRANSACTION", error))
        return false;

    std::string sql = "INSERT INTO workloads (workload_name, description) VALUES ('" +
                      escape(name) + "','" + escape(description) + "')";
    if (!execute(sql, error))
    {
        rollbackQuietly();
        return false;
    }
    workloadId = (long long)mysql_insert_id(conn);

    std::string values;
    for (size_t i = 0; i < processes.size(); i++)
    {
        const Process &p = processes[i];
        if (i)
            values += ",";
        values += "(" + toStr(workloadId) + ",'" + escape(p.pid) + "'," + toStr(p.arrivalTime) + "," +
                  toStr(p.burstTime) + "," + toStr(p.priority) + ")";
    }
    sql = "INSERT INTO processes (workload_id, process_id, arrival_time, burst_time, priority) VALUES " + values;
    if (!execute(sql, error))
    {
        rollbackQuietly();
        return false;
    }

    if (!execute("COMMIT", error))
    {
        rollbackQuietly();
        return false;
    }
    return true;
}

bool Database::listWorkloads(std::string &json, std::string &error)
{
    std::vector<std::string> rows;
    // JOIN + COUNT + GROUP BY + ORDER BY
    std::string sql =
        "SELECT w.workload_id, w.workload_name, w.description, w.created_at, COUNT(p.process_key) AS process_count "
        "FROM workloads w LEFT JOIN processes p ON p.workload_id = w.workload_id "
        "GROUP BY w.workload_id, w.workload_name, w.description, w.created_at "
        "ORDER BY w.workload_id DESC";
    if (!queryRows(sql, rows, error))
        return false;
    json = joinRows(rows);
    return true;
}

DbStatus Database::getWorkloadJson(long long id, std::string &json, std::string &error)
{
    std::vector<std::string> header, procs;
    if (!queryRows("SELECT workload_id, workload_name, description, created_at FROM workloads WHERE workload_id = " + toStr(id),
                   header, error))
        return DB_ERROR;
    if (header.empty())
    {
        error = "Workload not found.";
        return DB_NOT_FOUND;
    }

    // JOIN: workload + processes
    std::string sql =
        "SELECT p.process_id, p.arrival_time, p.burst_time, p.priority "
        "FROM workloads w JOIN processes p ON p.workload_id = w.workload_id "
        "WHERE w.workload_id = " +
        toStr(id) + " ORDER BY p.process_key";
    if (!queryRows(sql, procs, error))
        return DB_ERROR;

    json = "{\"workload\":" + header[0] + ",\"processes\":" + joinRows(procs) + "}";
    return DB_OK;
}

DbStatus Database::loadWorkload(long long id, std::string &name, std::vector<Process> &processes, std::string &error)
{
    std::vector<std::vector<std::string>> table;
    if (!queryTable("SELECT workload_name FROM workloads WHERE workload_id = " + toStr(id), table, error))
        return DB_ERROR;
    if (table.empty())
    {
        error = "The selected workload does not exist.";
        return DB_NOT_FOUND;
    }
    name = table[0][0];

    if (!queryTable("SELECT process_id, arrival_time, burst_time, priority FROM processes WHERE workload_id = " +
                        toStr(id) + " ORDER BY process_key",
                    table, error))
        return DB_ERROR;
    processes.clear();
    for (size_t i = 0; i < table.size(); i++)
    {
        Process p;
        p.pid = table[i][0];
        p.arrivalTime = atoi(table[i][1].c_str());
        p.burstTime = atoi(table[i][2].c_str());
        p.priority = atoi(table[i][3].c_str());
        processes.push_back(p);
    }
    return DB_OK;
}

// ------------------------------------------------------------------
// scheduling runs
// ------------------------------------------------------------------
bool Database::saveRun(long long workloadId, const SchedulingResult &r, long long &runId, std::string &error)
{
    if (!ensureConnected(error))
        return false;

    if (!execute("START TRANSACTION", error))
        return false;

    // 1. the run itself
    std::string sql =
        "INSERT INTO scheduling_runs (workload_id, algorithm, time_quantum, average_waiting_time, "
        "average_turnaround_time, average_response_time, throughput, cpu_utilization, context_switches) VALUES (" +
        toStr(workloadId) + ",'" + escape(r.algorithm) + "'," +
        (r.timeQuantum > 0 ? toStr(r.timeQuantum) : std::string("NULL")) + "," +
        num(r.avgWaitingTime, 4) + "," + num(r.avgTurnaroundTime, 4) + "," + num(r.avgResponseTime, 4) + "," +
        num(r.throughput, 6) + "," + num(r.cpuUtilization, 2) + "," + toStr(r.contextSwitches) + ")";
    if (!execute(sql, error))
    {
        rollbackQuietly();
        return false;
    }
    runId = (long long)mysql_insert_id(conn);

    // 2. one row per process
    std::string values;
    for (size_t i = 0; i < r.processes.size(); i++)
    {
        const Process &p = r.processes[i];
        if (i)
            values += ",";
        values += "(" + toStr(runId) + ",'" + escape(p.pid) + "'," + toStr(p.completionTime) + "," +
                  toStr(p.turnaroundTime) + "," + toStr(p.waitingTime) + "," + toStr(p.responseTime) + ")";
    }
    sql = "INSERT INTO scheduling_results (run_id, process_id, completion_time, turnaround_time, waiting_time, response_time) VALUES " + values;
    if (!execute(sql, error))
    {
        rollbackQuietly();
        return false;
    }

    // 3. Gantt chart blocks (including IDLE)
    values.clear();
    for (size_t i = 0; i < r.gantt.size(); i++)
    {
        const GanttSegment &g = r.gantt[i];
        if (i)
            values += ",";
        values += "(" + toStr(runId) + ",'" + escape(g.processId) + "'," + toStr(g.startTime) + "," + toStr(g.endTime) + ")";
    }
    sql = "INSERT INTO gantt_segments (run_id, process_id, start_time, end_time) VALUES " + values;
    if (!execute(sql, error))
    {
        rollbackQuietly();
        return false;
    }

    if (!execute("COMMIT", error))
    {
        rollbackQuietly();
        return false;
    }
    return true;
}

bool Database::listRuns(std::string &json, std::string &error)
{
    std::vector<std::string> rows;
    if (!queryRows("SELECT * FROM algorithm_run_summary ORDER BY created_at DESC, run_id DESC LIMIT 500", rows, error))
        return false;
    json = joinRows(rows);
    return true;
}

DbStatus Database::getRunJson(long long runId, std::string &json, std::string &error)
{
    std::vector<std::string> run, results, gantt;
    if (!queryRows("SELECT * FROM algorithm_run_summary WHERE run_id = " + toStr(runId), run, error))
        return DB_ERROR;
    if (run.empty())
    {
        error = "Experiment (run) not found.";
        return DB_NOT_FOUND;
    }

    // JOIN of three tables: run + results + the original process data
    std::string sql =
        "SELECT res.process_id, p.arrival_time, p.burst_time, p.priority, "
        "res.completion_time, res.turnaround_time, res.waiting_time, res.response_time "
        "FROM scheduling_runs s "
        "JOIN scheduling_results res ON res.run_id = s.run_id "
        "JOIN processes p ON p.workload_id = s.workload_id AND p.process_id = res.process_id "
        "WHERE s.run_id = " +
        toStr(runId) + " ORDER BY p.process_key";
    if (!queryRows(sql, results, error))
        return DB_ERROR;

    sql = "SELECT process_id, start_time, end_time FROM gantt_segments WHERE run_id = " + toStr(runId) +
          " ORDER BY start_time, segment_id";
    if (!queryRows(sql, gantt, error))
        return DB_ERROR;

    json = "{\"run\":" + run[0] + ",\"results\":" + joinRows(results) + ",\"gantt\":" + joinRows(gantt) + "}";
    return DB_OK;
}

bool Database::compareStored(long long workloadId, std::string &json, std::string &error)
{
    std::vector<std::string> rows;
    if (!queryRows("CALL sp_compare_algorithms(" + toStr(workloadId) + ")", rows, error))
        return false;
    json = joinRows(rows);
    return true;
}

bool Database::getStats(std::string &json, std::string &error)
{
    std::vector<std::string> totals, byAlgorithm, recent;

    if (!queryRows("SELECT (SELECT COUNT(*) FROM workloads) AS total_workloads, "
                   "(SELECT COUNT(*) FROM scheduling_runs) AS total_runs, "
                   "(SELECT COUNT(DISTINCT algorithm) FROM scheduling_runs) AS algorithms_used",
                   totals, error))
        return false;

    // aggregate functions + GROUP BY + ORDER BY
    std::string sql =
        "SELECT algorithm, COUNT(*) AS runs, "
        "ROUND(AVG(average_waiting_time), 2) AS avg_waiting, "
        "ROUND(MIN(average_waiting_time), 2) AS min_waiting, "
        "ROUND(MAX(average_waiting_time), 2) AS max_waiting, "
        "ROUND(AVG(average_turnaround_time), 2) AS avg_turnaround, "
        "ROUND(AVG(cpu_utilization), 2) AS avg_cpu_utilization, "
        "SUM(context_switches) AS total_context_switches "
        "FROM scheduling_runs GROUP BY algorithm ORDER BY algorithm";
    if (!queryRows(sql, byAlgorithm, error))
        return false;

    if (!queryRows("SELECT * FROM algorithm_run_summary ORDER BY created_at DESC, run_id DESC LIMIT 5", recent, error))
        return false;

    json = "{\"totals\":" + totals[0] + ",\"by_algorithm\":" + joinRows(byAlgorithm) + ",\"recent\":" + joinRows(recent) + "}";
    return true;
}
