# CPU Scheduling Performance Analysis

## Objective
The objective of this report is to compare CPU scheduling algorithms using standard performance metrics.

## Algorithms
- First Come First Serve (FCFS)
- Shortest Job First (SJF)
- Shortest Remaining Time First (SRTF)
- Priority Scheduling
- Round Robin (RR)

## Performance Metrics

| Metric | Description |
|---|---|
| Completion Time (CT) | Time at which a process finishes execution |
| Turnaround Time (TAT) | Total time from process arrival to completion |
| Waiting Time (WT) | Total time a process spends waiting |
| Response Time (RT) | Time between process arrival and its first CPU execution |
| Throughput | Number of completed processes per unit of time |
| CPU Utilization | Percentage of time the CPU is busy |
| Context Switches | Number of transitions between processes |

## Formulas

- Turnaround Time = Completion Time - Arrival Time
- Waiting Time = Turnaround Time - Burst Time
- Response Time = First Start Time - Arrival Time
- Throughput = Number of Completed Processes / Total Time
- CPU Utilization = (Busy Time / Total Time) × 100

## Comparison Method

All five algorithms should be evaluated using the same process workload. Their average waiting time, average turnaround time, average response time, throughput, CPU utilization, and context switches should then be compared.

## Conclusion

No single scheduling algorithm performs best for every workload. The results depend on process arrival times, burst times, priorities, and the Round Robin time quantum.