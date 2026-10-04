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

