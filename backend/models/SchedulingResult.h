// SchedulingResult.h - everything produced by one scheduling run
#ifndef SCHEDULING_RESULT_H
#define SCHEDULING_RESULT_H

#include <string>
#include <vector>
#include "Process.h"
#include "GanttSegment.h"

struct SchedulingResult
{
    std::string algorithm; // FCFS, SJF, SRTF, RR, PRIORITY
    int timeQuantum = 0;   // only used by RR (0 = not applicable)

    std::vector<Process> processes;  // same order as the input, with results filled in
    std::vector<GanttSegment> gantt; // chronological, includes IDLE blocks

    double avgWaitingTime = 0;
    double avgTurnaroundTime = 0;
    double avgResponseTime = 0;
    double throughput = 0;     // processes per time unit
    double cpuUtilization = 0; // percent
    int contextSwitches = 0;

    int totalTime = 0; // time at which the last process finished
    int busyTime = 0;  // time the CPU spent running processes
};

#endif