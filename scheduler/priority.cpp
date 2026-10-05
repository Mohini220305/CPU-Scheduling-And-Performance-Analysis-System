// priority.cpp - Priority scheduling (non-preemptive)
// Tie-breaking: if two or more processes have equal priority, then the process with earlier arrival time gets earlier position in the input list.
#include "scheduler.h"

SchedulingResult runPriority(const std::vector<Process> &input)
{
    SchedulingResult result;
    result.algorithm = "PRIORITY";
    result.processes = input;
    prepareProcesses(result.processes);
    std::vector<Process> &p = result.processes;
    int n = (int)p.size();

    std::vector<bool> done(n, false);
    int completed = 0, time = 0;

    while (completed < n)
    {
        int pick = -1;
        for (int i = 0; i < n; i++)
        {
            if (done[i] || p[i].arrivalTime > time)
                continue;
            if (pick == -1 || p[i].priority < p[pick].priority ||
                (p[i].priority == p[pick].priority && p[i].arrivalTime < p[pick].arrivalTime))
            {
                pick = i;
            }
        }

        if (pick == -1)
        { // CPU idle until the next arrival
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
