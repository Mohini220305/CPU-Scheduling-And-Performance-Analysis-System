// scheduler.h - public interface of the CPU scheduling engine
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <vector>
#include "../backend/models/SchedulingResult.h"

// ---------------- the five algorithms ----------------
SchedulingResult runFCFS(const std::vector<Process> &input);                       
SchedulingResult runSJF(const std::vector<Process> &input);                         
SchedulingResult runSRTF(const std::vector<Process> &input);                        
SchedulingResult runRoundRobin(const std::vector<Process> &input, int timeQuantum); 
SchedulingResult runPriority(const std::vector<Process> &input);                    

// ---------------- helpers (scheduler_common.cpp) ----------------

// Limits used for validation (also documented in the README)
const int SCHED_MAX_PROCESSES = 50;
const int SCHED_MAX_ARRIVAL = 1000;
const int SCHED_MAX_BURST = 1000;
const int SCHED_MIN_PRIORITY = 1;
const int SCHED_MAX_PRIORITY = 100;
const int SCHED_MAX_QUANTUM = 1000;

// Checks a list of processes. Returns false and fills 'error' on the first problem.
bool validateProcesses(const std::vector<Process> &processes, std::string &error);

// Runs the algorithm called 'algorithm' ("FCFS","SJF","SRTF","RR","PRIORITY").
// Returns false and fills 'error' if the algorithm / quantum / processes are invalid.
bool runScheduler(const std::string &algorithm, const std::vector<Process> &processes,
                  int timeQuantum, SchedulingResult &out, std::string &error);

// Adds [start,end) to the Gantt chart; merges with the previous block if it is the same process.
void addSegment(std::vector<GanttSegment> &gantt, const std::string &pid, int start, int end);

// Resets remaining/first-start values before an algorithm starts.
void prepareProcesses(std::vector<Process> &processes);

// Calculates per-process metrics, averages, throughput, utilisation and context switches.
void finalizeResult(SchedulingResult &result);

#endif
