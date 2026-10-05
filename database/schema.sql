CREATE DATABASE IF NOT EXISTS cpu_scheduling_db
    CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci;

USE cpu_scheduling_db;


-- 1. workloads : one row per workload (a set of processes)
CREATE TABLE IF NOT EXISTS workloads (
    workload_id   INT           NOT NULL AUTO_INCREMENT,
    workload_name VARCHAR(100)  NOT NULL,
    description   VARCHAR(500)  NOT NULL DEFAULT '',
    created_at    DATETIME      NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (workload_id)
) ENGINE = InnoDB;

-- 2. processes : Workload 1 ---- N Processes
-- process_key is the primary key (also keeps the order of entry);
-- (workload_id, process_id) is UNIQUE so a PID cannot repeat inside a workload.

CREATE TABLE IF NOT EXISTS processes (
    process_key   INT          NOT NULL AUTO_INCREMENT,
    workload_id   INT          NOT NULL,
    process_id    VARCHAR(20)  NOT NULL,
    arrival_time  INT          NOT NULL,
    burst_time    INT          NOT NULL,
    priority      INT          NOT NULL,
    PRIMARY KEY (process_key),
    CONSTRAINT uq_process_in_workload UNIQUE (workload_id, process_id),   
    
    CONSTRAINT fk_processes_workload FOREIGN KEY (workload_id)
        REFERENCES workloads (workload_id) ON DELETE CASCADE,
    CONSTRAINT chk_arrival  CHECK (arrival_time >= 0),
    CONSTRAINT chk_burst    CHECK (burst_time > 0),
    CONSTRAINT chk_priority CHECK (priority BETWEEN 1 AND 100)
) ENGINE = InnoDB;

-- 3. scheduling_runs : Workload 1 ---- N Runs  (one row per experiment)
--    time_quantum is NULL for every algorithm except RR.
CREATE TABLE IF NOT EXISTS scheduling_runs (
    run_id                  INT            NOT NULL AUTO_INCREMENT,
    workload_id             INT            NOT NULL,
    algorithm               ENUM('FCFS','SJF','SRTF','RR','PRIORITY') NOT NULL,
    time_quantum            INT            NULL,
    average_waiting_time    DECIMAL(10,4)  NOT NULL,
    average_turnaround_time DECIMAL(10,4)  NOT NULL,
    average_response_time   DECIMAL(10,4)  NOT NULL,
    throughput              DECIMAL(12,6)  NOT NULL,
    cpu_utilization         DECIMAL(6,2)   NOT NULL,
    context_switches        INT            NOT NULL,
    created_at              DATETIME       NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (run_id),
    CONSTRAINT fk_runs_workload FOREIGN KEY (workload_id)
        REFERENCES workloads (workload_id) ON DELETE CASCADE,
    CONSTRAINT chk_quantum CHECK (time_quantum IS NULL OR time_quantum > 0),
    INDEX idx_runs_workload_algorithm (workload_id, algorithm),   -- comparison queries
    INDEX idx_runs_algorithm (algorithm),                         -- GROUP BY algorithm
    INDEX idx_runs_created_at (created_at)                        -- experiment history ORDER BY
) ENGINE = InnoDB;


-- 4. scheduling_results : Run 1 ---- N per-process results
CREATE TABLE IF NOT EXISTS scheduling_results (
    result_id       INT          NOT NULL AUTO_INCREMENT,
    run_id          INT          NOT NULL,
    process_id      VARCHAR(20)  NOT NULL,
    completion_time INT          NOT NULL,
    turnaround_time INT          NOT NULL,
    waiting_time    INT          NOT NULL,
    response_time   INT          NOT NULL,
    PRIMARY KEY (result_id),
    CONSTRAINT fk_results_run FOREIGN KEY (run_id)
        REFERENCES scheduling_runs (run_id) ON DELETE CASCADE,
    INDEX idx_results_run_process (run_id, process_id)
) ENGINE = InnoDB;

-- 5. gantt_segments : Run 1 ---- N Gantt blocks
--    CPU idle time is stored as process_id = 'IDLE'.
CREATE TABLE IF NOT EXISTS gantt_segments (
    segment_id  INT          NOT NULL AUTO_INCREMENT,
    run_id      INT          NOT NULL,
    process_id  VARCHAR(20)  NOT NULL,
    start_time  INT          NOT NULL,
    end_time    INT          NOT NULL,
    PRIMARY KEY (segment_id),
    CONSTRAINT fk_gantt_run FOREIGN KEY (run_id)
        REFERENCES scheduling_runs (run_id) ON DELETE CASCADE,
    CONSTRAINT chk_gantt_times CHECK (end_time > start_time),
    INDEX idx_gantt_run_start (run_id, start_time)
) ENGINE = InnoDB;

-- VIEW : algorithm_run_summary
-- One convenient row per run: JOIN of runs + workloads + count of results.
DROP VIEW IF EXISTS algorithm_run_summary;
CREATE VIEW algorithm_run_summary AS
SELECT  r.run_id,
        r.workload_id,
        w.workload_name,
        r.algorithm,
        r.time_quantum,
        r.average_waiting_time,
        r.average_turnaround_time,
        r.average_response_time,
        r.throughput,
        r.cpu_utilization,
        r.context_switches,
        r.created_at,
        (SELECT COUNT(*) FROM scheduling_results sr WHERE sr.run_id = r.run_id) AS process_count
FROM    scheduling_runs r
JOIN    workloads w ON w.workload_id = r.workload_id;

-- STORED PROCEDURE : sp_compare_algorithms(workload_id)
-- Returns the LATEST run of every algorithm for one workload.
-- (JOIN + derived table + MAX() + GROUP BY + ORDER BY)
DROP PROCEDURE IF EXISTS sp_compare_algorithms;
DELIMITER $$
CREATE PROCEDURE sp_compare_algorithms(IN p_workload_id INT)
BEGIN
    SELECT  r.run_id,
            r.algorithm,
            r.time_quantum,
            r.average_waiting_time,
            r.average_turnaround_time,
            r.average_response_time,
            r.throughput,
            r.cpu_utilization,
            r.context_switches,
            r.created_at
    FROM    scheduling_runs r
    JOIN   (SELECT algorithm, MAX(run_id) AS latest_run_id
            FROM   scheduling_runs
            WHERE  workload_id = p_workload_id
            GROUP BY algorithm) latest
           ON latest.latest_run_id = r.run_id
    ORDER BY r.algorithm; 
END$$
DELIMITER ;

