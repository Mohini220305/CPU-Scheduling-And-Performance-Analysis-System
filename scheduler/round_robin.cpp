// round_robin.cpp - Round Robin (preemptive, time quantum given by the user)
#include "scheduler.h"
#include <algorithm>
#include <numeric>
#include <queue>

SchedulingResult runRoundRobin(const std::vector<Process> &input, int timeQuantum)
{
    SchedulingResult result;
    result.algorithm = "RR";
    result.timeQuantum = timeQuantum;
    result.processes = input;
    prepareProcesses(result.processes);
    std::vector<Process> &p = result.processes;
    int n = (int)p.size();

    // indices sorted by arrival time (equal arrival -> input order)
    std::vector<int> byArrival(n);
    std::iota(byArrival.begin(), byArrival.end(), 0);
    std::stable_sort(byArrival.begin(), byArrival.end(),
                     [&](int a, int b)
                     { return p[a].arrivalTime < p[b].arrivalTime; });

    std::queue<int> ready; // the ready queue
    int nextToArrive = 0;  // position in byArrival of the next process that has not arrived yet
    int completed = 0, time = 0;

    while (completed < n)
    {
        while (nextToArrive < n && p[byArrival[nextToArrive]].arrivalTime <= time)
        {
            ready.push(byArrival[nextToArrive]);
            nextToArrive++;
        }

        if (ready.empty())
        { 
            int nextArrival = p[byArrival[nextToArrive]].arrivalTime;
            addSegment(result.gantt, IDLE_PID, time, nextArrival);
            time = nextArrival;
            continue;
        }

        int cur = ready.front();
        ready.pop();

        if (p[cur].firstStartTime == -1)
            p[cur].firstStartTime = time;

        int slice = std::min(timeQuantum, p[cur].remainingTime);
        addSegment(result.gantt, p[cur].pid, time, time + slice);
        time += slice;
        p[cur].remainingTime -= slice;

        while (nextToArrive < n && p[byArrival[nextToArrive]].arrivalTime <= time)
        {
            ready.push(byArrival[nextToArrive]);
            nextToArrive++;
        }

        if (p[cur].remainingTime > 0)
        {
            ready.push(cur);
        }
        else
        {
            p[cur].completionTime = time;
            completed++;
        }
    }

    finalizeResult(result);
    return result;
}
