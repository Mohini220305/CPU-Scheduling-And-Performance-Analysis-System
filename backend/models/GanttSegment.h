// GanttSegment.h - one block of the Gantt chart
#ifndef GANTT_SEGMENT_H
#define GANTT_SEGMENT_H

#include <string>

// Name used for CPU idle blocks 
#define IDLE_PID "IDLE"

struct GanttSegment
{
    std::string processId; // process id or "IDLE"
    int startTime = 0;
    int endTime = 0;
};

#endif
