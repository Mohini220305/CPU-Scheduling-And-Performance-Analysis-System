// srtf.cpp - Shortest Remaining Time First (preemptive)
#include "scheduler.h"

SchedulingResult runSRTF(const std::vector<Process> &input)
{
    SchedulingResult result;
    result.algorithm = "SRTF";
    result.processes = input;
    prepareProcesses(result.processes);
    std::vector<Process> &p = result.processes;
    int n = (int)p.size();

    int completed = 0, time = 0;
    int running = -1; 

    while (completed < n)
    {
        int pick = -1;
        for (int i = 0; i < n; i++)
        {
            if (p[i].remainingTime == 0 || p[i].arrivalTime > time)
                continue;
            if (pick == -1)
            {
                pick = i;
                continue;
            }
            if (p[i].remainingTime < p[pick].remainingTime)
            {
                pick = i;
            }
            else if (p[i].remainingTime == p[pick].remainingTime)
            {
                if (i == running)
                    pick = i; // keep the running process
                else if (pick != running && p[i].arrivalTime < p[pick].arrivalTime)
                    pick = i;
            }
        }

        if (pick == -1)
        { 
            int nextArrival = -1;
            for (int i = 0; i < n; i++)
                if (p[i].remainingTime > 0 && (nextArrival == -1 || p[i].arrivalTime < nextArrival))
                    nextArrival = p[i].arrivalTime;
            addSegment(result.gantt, IDLE_PID, time, nextArrival);
            time = nextArrival;
            running = -1;
            continue;
        }

        if (p[pick].firstStartTime == -1)
            p[pick].firstStartTime = time;

        addSegment(result.gantt, p[pick].pid, time, time + 1); 
        p[pick].remainingTime--;
        time++;
        running = pick;

        if (p[pick].remainingTime == 0)
        {
            p[pick].completionTime = time;
            completed++;
            running = -1;
        }
    }

    finalizeResult(result);
    return result;
}
