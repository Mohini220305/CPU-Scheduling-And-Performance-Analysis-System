// sjf.cpp - Shortest Job First (non-preemptive)
// Tie-breaking: if two processes have equal burst time, then earlier arrival time process gets earlier position in the input list.
#include "scheduler.h"

SchedulingResult runSJF(const std::vector<Process> &input)
{
    SchedulingResult result;
    result.algorithm = "SJF";
    result.processes = input;
    prepareProcesses(result.processes);
    std::vector<Process> &p = result.processes;
    int n = (int)p.size();

    std::vector<bool> done(n, false);
    int completed = 0, time = 0;

    while (completed < n)
    {
        // choose among the processes that have already arrived
        int pick = -1;
        for (int i = 0; i < n; i++)
        {
            if (done[i] || p[i].arrivalTime > time)
                continue;
            if (pick == -1 || p[i].burstTime < p[pick].burstTime ||
                (p[i].burstTime == p[pick].burstTime && p[i].arrivalTime < p[pick].arrivalTime))
            {
                pick = i;
            }
        }

        if (pick == -1)
        { // nobody is ready -> CPU idle
            int nextArrival = -1;
            for (int i = 0; i < n; i++)
                if (!done[i] && (nextArrival == -1 || p[i].arrivalTime < nextArrival))
                    nextArrival = p[i].arrivalTime;
            addSegment(result.gantt, IDLE_PID, time, nextArrival);
            time = nextArrival;
            continue;
        }

        p[pick].firstStartTime = time;
        addSegment(result.gantt, p[pick].pid, time, time + p[pick].burstTime);
        time += p[pick].burstTime;
        p[pick].remainingTime = 0;
        p[pick].completionTime = time;
        done[pick] = true;
        completed++;
    }

    finalizeResult(result);
    return result;
}
