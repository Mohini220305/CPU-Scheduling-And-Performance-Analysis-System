#include <iostream>
#include <vector>
#include <iomanip>
#include "scheduler/scheduler.h"

using namespace std;

int main()
{
    // Sample processes
    vector<Process> processes = {
        {"P1", 0, 5, 2},
        {"P2", 1, 3, 1},
        {"P3", 2, 8, 4},
        {"P4", 3, 6, 2}};

    // Run FCFS
    SchedulingResult result = runFCFS(processes);

    cout << "\n========== FCFS CPU SCHEDULING ==========\n\n";

    // ---------------- Gantt Chart ----------------
    cout << "Gantt Chart:\n";

    for (const auto &segment : result.gantt)
    {
        cout << "| " << segment.processId << " ";
    }

    cout << "|\n";

    cout << "0";

    for (const auto &segment : result.gantt)
    {
        cout << "\t" << segment.endTime;
    }

    cout << "\n\n";

    // ---------------- Process Metrics ----------------
    cout << "Process Metrics:\n\n";

    cout << "PID\tAT\tBT\tCT\tTAT\tWT\tRT\n";
    cout << "------------------------------------------------\n";

    for (const auto &p : result.processes)
    {
        cout << p.pid << "\t"
             << p.arrivalTime << "\t"
             << p.burstTime << "\t"
             << p.completionTime << "\t"
             << p.turnaroundTime << "\t"
             << p.waitingTime << "\t"
             << p.responseTime << "\n";
    }

    cout << fixed << setprecision(2);

    cout << "\nAverage Waiting Time    : "
         << result.avgWaitingTime << "\n";

    cout << "Average Turnaround Time : "
         << result.avgTurnaroundTime << "\n";

    cout << "Average Response Time   : "
         << result.avgResponseTime << "\n";

    cout << "Throughput              : "
         << result.throughput << "\n";

    cout << "CPU Utilization         : "
         << result.cpuUtilization << "%\n";

    cout << "Context Switches        : "
         << result.contextSwitches << "\n";

    cout << "\n==========================================\n";

    return 0;
}