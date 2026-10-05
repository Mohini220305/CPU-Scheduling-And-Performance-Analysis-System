// scheduler_common.cpp - validation, Gantt helper, metric calculation, algorithm dispatcher
#include "scheduler.h"
#include <cctype>
#include <exception>
#include <set>

static std::string toUpper(std::string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)std::toupper((unsigned char)s[i]);
    return s;
}

bool validateProcesses(const std::vector<Process>& processes, std::string& error) {
    if (processes.empty()) {
        error = "At least one process is required.";
        return false;
    }
    if ((int)processes.size() > SCHED_MAX_PROCESSES) {
        error = "A workload can contain at most " + std::to_string(SCHED_MAX_PROCESSES) + " processes.";
        return false;
    }
    std::set<std::string> seen;  // upper-case ids, because MySQL compares ids case-insensitively
    for (size_t i = 0; i < processes.size(); i++) {
        const Process& p = processes[i];
        if (p.pid.empty()) {
            error = "Process #" + std::to_string(i + 1) + " has an empty Process ID.";
            return false;
        }
        if (p.pid.size() > 20) {
            error = "Process ID '" + p.pid + "' is too long (maximum 20 characters).";
            return false;
        }
        for (size_t k = 0; k < p.pid.size(); k++) {
            unsigned char c = (unsigned char)p.pid[k];
            if (!(std::isalnum(c) || c == '_' || c == '-')) {
                error = "Process ID '" + p.pid + "' may only contain letters, digits, '_' and '-'.";
                return false;
            }
        }
        std::string key = toUpper(p.pid);
        if (key == IDLE_PID) {
            error = "'IDLE' is reserved for CPU idle time and cannot be used as a Process ID.";
            return false;
        }
        if (seen.count(key)) {
            error = "Duplicate Process ID '" + p.pid + "'. Process IDs must be unique within a workload.";
            return false;
        }
        seen.insert(key);
        if (p.arrivalTime < 0) {
            error = "Process " + p.pid + ": arrival time cannot be negative.";
            return false;
        }
        if (p.arrivalTime > SCHED_MAX_ARRIVAL) {
            error = "Process " + p.pid + ": arrival time cannot be more than " + std::to_string(SCHED_MAX_ARRIVAL) + ".";
            return false;
        }
        if (p.burstTime <= 0) {
            error = "Process " + p.pid + ": burst time must be greater than 0.";
            return false;
        }
        if (p.burstTime > SCHED_MAX_BURST) {
            error = "Process " + p.pid + ": burst time cannot be more than " + std::to_string(SCHED_MAX_BURST) + ".";
            return false;
        }
        if (p.priority < SCHED_MIN_PRIORITY || p.priority > SCHED_MAX_PRIORITY) {
            error = "Process " + p.pid + ": priority must be a whole number from " + std::to_string(SCHED_MIN_PRIORITY) +
                    " to " + std::to_string(SCHED_MAX_PRIORITY) + " (1 = highest priority).";
            return false;
        }
    }
    return true;
}

void addSegment(std::vector<GanttSegment>& gantt, const std::string& pid, int start, int end) {
    if (end <= start) return;  // empty block
    if (!gantt.empty() && gantt.back().processId == pid && gantt.back().endTime == start) {
        gantt.back().endTime = end;  // same process continues -> extend the block
        return;
    }
    GanttSegment seg;
    seg.processId = pid;
    seg.startTime = start;
    seg.endTime = end;
    gantt.push_back(seg);
}

void prepareProcesses(std::vector<Process>& processes) {
    for (size_t i = 0; i < processes.size(); i++) {
        processes[i].remainingTime = processes[i].burstTime;
        processes[i].completionTime = 0;
        processes[i].turnaroundTime = 0;
        processes[i].waitingTime = 0;
        processes[i].responseTime = 0;
        processes[i].firstStartTime = -1;
    }
}

void finalizeResult(SchedulingResult& r) {
    double sumWT = 0, sumTAT = 0, sumRT = 0;
    int lastCompletion = 0;

    for (size_t i = 0; i < r.processes.size(); i++) {
        Process& p = r.processes[i];
        p.turnaroundTime = p.completionTime - p.arrivalTime;   // TAT = CT - AT
        p.waitingTime = p.turnaroundTime - p.burstTime;        // WT  = TAT - BT
        p.responseTime = p.firstStartTime - p.arrivalTime;     // RT  = first start - AT
        sumWT += p.waitingTime;
        sumTAT += p.turnaroundTime;
        sumRT += p.responseTime;
        if (p.completionTime > lastCompletion) lastCompletion = p.completionTime;
    }

    int busy = 0;
    for (size_t i = 0; i < r.gantt.size(); i++) {
        if (r.gantt[i].processId != IDLE_PID) busy += r.gantt[i].endTime - r.gantt[i].startTime;
    }

    int switches = 0;
    std::string lastPid = "";
    for (size_t i = 0; i < r.gantt.size(); i++) {
        if (r.gantt[i].processId == IDLE_PID) continue;
        if (!lastPid.empty() && r.gantt[i].processId != lastPid) switches++;
        lastPid = r.gantt[i].processId;
    }

    double n = (double)r.processes.size();
    r.totalTime = lastCompletion;   // the clock starts at time 0
    r.busyTime = busy;
    r.avgWaitingTime = n > 0 ? sumWT / n : 0;
    r.avgTurnaroundTime = n > 0 ? sumTAT / n : 0;
    r.avgResponseTime = n > 0 ? sumRT / n : 0;
    r.throughput = lastCompletion > 0 ? n / lastCompletion : 0;
    r.cpuUtilization = lastCompletion > 0 ? (100.0 * busy) / lastCompletion : 0;
    r.contextSwitches = switches;
}

bool runScheduler(const std::string& algorithm, const std::vector<Process>& processes,
                  int timeQuantum, SchedulingResult& out, std::string& error) {
    if (!validateProcesses(processes, error)) return false;
    try {
        if (algorithm == "FCFS") {
            out = runFCFS(processes);
        } else if (algorithm == "SJF") {
            out = runSJF(processes);
        } else if (algorithm == "SRTF") {
            out = runSRTF(processes);
        } /*else if (algorithm == "RR") {
            if (timeQuantum <= 0) {
                error = "Time quantum must be greater than 0 for Round Robin.";
                return false;
            }
            if (timeQuantum > SCHED_MAX_QUANTUM) {
                error = "Time quantum cannot be more than " + std::to_string(SCHED_MAX_QUANTUM) + ".";
                return false;
            }
            out = runRoundRobin(processes, timeQuantum);
        } else if (algorithm == "PRIORITY") {
            out = runPriority(processes);
        } */else {
            error = "Unknown scheduling algorithm '" + algorithm + "'.";
            return false;
        }
    } catch (const std::exception& e) {
        error = std::string("Scheduling failed: ") + e.what();
        return false;
    }
    return true;
}
