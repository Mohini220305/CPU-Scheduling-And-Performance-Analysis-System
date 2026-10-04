// fcfs.cpp - First Come First Serve (non-preemptive)
#include "scheduler.h"
#include <algorithm>
#include <numeric>

SchedulingResult runFCFS(const std::vector<Process>& input) {
    SchedulingResult result;
    result.algorithm = "FCFS";
    result.processes = input;
    prepareProcesses(result.processes);
    std::vector<Process>& p = result.processes;
    int n = (int)p.size();

    // Execution order = arrival time; equal arrival -> the order they were entered (stable sort)
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&](int a, int b) { return p[a].arrivalTime < p[b].arrivalTime; });

    int time = 0;
    for (int k = 0; k < n; k++) {
        Process& cur = p[order[k]];
        if (time < cur.arrivalTime) {                       // CPU idle until the process arrives
            addSegment(result.gantt, IDLE_PID, time, cur.arrivalTime);
            time = cur.arrivalTime;
        }
        cur.firstStartTime = time;
        addSegment(result.gantt, cur.pid, time, time + cur.burstTime);
        time += cur.burstTime;
        cur.remainingTime = 0;
        cur.completionTime = time;
    }

    finalizeResult(result);
    return result;
}
