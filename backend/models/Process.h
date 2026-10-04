// Process.h - one process of a workload (input + calculated results)
#ifndef PROCESS_H
#define PROCESS_H

#include <string>

struct Process
{
    std::string pid;     // process name, e.g. "P1"
    int arrivalTime = 0; // time at which the process enters the ready queue
    int burstTime = 0;   // total CPU time needed
    int priority = 1;    // lower number = higher priority

    int remainingTime = 0;   // CPU time still needed
    int completionTime = 0;  // time at which the process finished
    int turnaroundTime = 0;  // completion - arrival
    int waitingTime = 0;     // turnaround - burst
    int responseTime = 0;    // first start - arrival
    int firstStartTime = -1; // first time the process got the CPU (-1 = not yet)
};

#endif
