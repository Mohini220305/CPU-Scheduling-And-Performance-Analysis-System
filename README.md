# CPU Scheduling and Performance Analysis System

## Project Description

The CPU Scheduling and Performance Analysis System is a database-driven project that focuses on the implementation and analysis of CPU scheduling algorithms.

The system aims to combine concepts of Operating Systems and Database Management Systems by storing process and scheduling information in a structured database and using the stored data for performance analysis.

The project will initially focus on traditional CPU scheduling algorithms and will later incorporate workload/dataset-based analysis as part of the project development.

---

## Problem Statement

CPU scheduling is an important function of an operating system that determines the order in which processes are executed by the CPU.

Different scheduling algorithms can produce different waiting time, turnaround time, response time, and CPU utilization. Comparing these algorithms manually becomes difficult when the number of processes increases.

The objective of this project is to develop a system that can simulate CPU scheduling algorithms, store process and scheduling information using a database, and analyze the performance of different scheduling approaches.

---

## Objectives

- Implement commonly used CPU scheduling algorithms.
- Generate or accept process/workload information.
- Calculate scheduling performance parameters such as:
  - Waiting Time
  - Turnaround Time
  - Response Time
  - Completion Time
- Store process and scheduling results in a database.
- Use database operations for meaningful analysis of scheduling results.
- Compare the performance of different scheduling algorithms.
- Provide graphical/visual representation of scheduling results.
- Explore dataset-based analysis and machine learning as the project progresses.

---

## Team Members

| Name | Role |
|------|------|
| Mohini Yadav | Team Lead |
| Anu Yadav | Team Member |
| Kanika Rawat | Team Member |

---

## Technologies and Tools

### Planned Technologies

- Programming Language: C++
- Database: MySQL
- Git and GitHub
- Visual Studio Code
- Operating System concepts and CPU scheduling algorithms
- Dataset/ML tools for later project development

---

## Major Modules

### 1. Process and Workload Management

This module will handle process information such as process ID, arrival time, burst time, priority, and other relevant attributes.

### 2. CPU Scheduling

The system will implement and analyze scheduling algorithms such as:

- First Come First Serve (FCFS)
- Shortest Job First (SJF)
- Round Robin (RR)
- Priority Scheduling

Additional algorithms may be considered during development.

### 3. Database Management

The database will store process information, scheduling configurations, and scheduling results.

SQL queries, database operations, and other DBMS features will be used for meaningful processing and analysis of the project data.

### 4. Performance Analysis

The system will calculate and compare scheduling parameters including:

- Waiting Time
- Turnaround Time
- Response Time
- Completion Time

### 5. Visualization

Scheduling results will be represented using suitable graphs and Gantt charts to make performance comparisons easier to understand.

### 6. Dataset and Machine Learning

A suitable dataset related to CPU scheduling/workloads will be explored during development.

Machine learning-based analysis may be incorporated after the basic scheduling and database components have been implemented and evaluated.

---

## Project Structure

```text
CPU-Scheduling-And-Performance-Analysis-System/
│
├── README.md
│
├── database/
│
├── src/
│   ├── scheduling/
│   ├── database/
│   └── workload/
│
├── tests/
│
└── docs/