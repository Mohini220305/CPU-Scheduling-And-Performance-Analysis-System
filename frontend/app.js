const API = "";



function $(id) {
    return document.getElementById(id);
}


// Show message
function showMessage(message, type) {
    const box = $("message");
    box.textContent = message;
    box.className = "message " + type;
}

// Add process row

function addProcess(pid = "", arrival = 0, burst = 1, priority = 1) {
    const table = $("processTable");
    const row = document.createElement("tr");
    row.innerHTML = `
        <td>
            <input
                type="text"
                class="pid"
                value="${pid}"
                placeholder="P1"
            >
        </td>

        <td>
            <input
                type="number"
                class="arrival"
                value="${arrival}"
                min="0"
                max="1000"
            >
        </td>

        <td>
            <input
                type="number"
                class="burst"
                value="${burst}"
                min="1"
                max="1000"
            >
        </td>

        <td>
            <input
                type="number"
                class="priority"
                value="${priority}"
                min="1"
                max="100"
            >
        </td>

        <td>

            <button
                class="danger remove-process"
            >
                Remove
            </button>

        </td>
    `;

    table.appendChild(row);
}


// Remove process

$("processTable").addEventListener("click", function (event) {
    if (event.target.classList.contains("remove-process")) {
        event.target
            .closest("tr")
            .remove();
    }
});

// Add process button

$("addProcess").addEventListener("click", function () {
    const count =
        $("processTable").querySelectorAll("tr").length;

    const pid = "P" + (count + 1);
    addProcess(pid, 0, 1, count + 1);
});


// Clear processes
$("clearProcesses").addEventListener("click", function () {
    $("processTable").innerHTML = "";
    addProcess("P1", 0, 5, 2);
    addProcess("P2", 1, 3, 1);
    addProcess("P3", 2, 8, 4);
    addProcess("P4", 3, 6, 2);
});


// Show/hide quantum
$("algorithm").addEventListener("change", function () {

    const algorithm = $("algorithm").value;
    if (algorithm === "RR") {
        $("quantumField").classList.remove("hidden");
    } else {
        $("quantumField").classList.add("hidden");
    }
});



// Collect process input
function getProcesses() {
    const rows =
        $("processTable").querySelectorAll("tr");
    const processes = [];
    if (rows.length === 0) {
        throw new Error(
            "Please add at least one process."
        );
    }

    rows.forEach(function (row, index) {
        const pid = row.querySelector(".pid").value.trim();
        const arrival = Number(row.querySelector(".arrival").value);
        const burst = Number(row.querySelector(".burst").value);
        const priority = Number(row.querySelector(".priority").value);

        if (pid === "") {
            throw new Error(
                "Process " + (index + 1) +    " must have a PID."
            );
        }

        if (arrival < 0 || arrival > 1000) {
            throw new Error(
                "Arrival time must be between 0 and 1000."
            );
        }

        if (burst <= 0 || burst > 1000) {
            throw new Error(
                "Burst time must be between 1 and 1000."
            );
        }

        if (priority < 1 || priority > 100) {
            throw new Error(
                "Priority must be between 1 and 100."
            );
        }

        processes.push({
            pid: pid,
            arrivalTime: arrival,
            burstTime: burst,
            priority: priority
        });
    });
    return processes;
}

// Run algorithm

async function runAlgorithm() {
    try {
        const algorithm = $("algorithm").value;
        const processes = getProcesses();
        const request = {
            algorithm: algorithm,
            processes: processes
        };

        // Round Robin requires quantum
        if (algorithm === "RR") {
            const quantum = Number($("quantum").value);
            if (quantum <= 0) {
                throw new Error(
                    "Time quantum must be greater than 0."
                );
            }
            request.timeQuantum = quantum;
        }

        $("runButton").disabled = true;

        showMessage(
            "Running " + algorithm + "...", "success"
        );

        console.log(
            "Sending request to C++:",request
        );

        // Send data to C++
        const response = await fetch(
            API + "/api/simulate",
            {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"

                },
                body: JSON.stringify(request)
            }
        );


        if (!response.ok) {
            throw new Error("Server returned HTTP " +response.status);
        }

        const body = await response.json();


        console.log("Response from C++:", body);

        if (!body.ok) {
            throw new Error( body.error || "Scheduling failed.");
        }

        displayResult(body.data);
        showMessage(algorithm +" scheduling completed successfully.","success");

    } catch (error) {
        console.error(error);
        showMessage(error.message, "error");
    } finally {
        $("runButton").disabled = false;
    }
}

// Display result
function displayResult(data) {
    $("resultSection") .classList .remove("hidden");
    const run = data.run;

    $("algorithmUsed").innerHTML = "<p><b>Algorithm:</b> " + escapeHtml(run.algorithm) + "</p>";
    $("avgWaiting").textContent = Number(run.average_waiting_time).toFixed(2);
    $("avgTurnaround").textContent = Number(run.average_turnaround_time).toFixed(2);
    $("avgResponse").textContent = Number(run.average_response_time).toFixed(2);
    $("throughput").textContent = Number(run.throughput).toFixed(4);
    $("cpuUtilization").textContent = Number(run.cpu_utilization).toFixed(2) + "%";
    $("contextSwitches").textContent = run.context_switches;

    const table = $("resultTable");

    table.innerHTML = "";

    data.results.forEach(function (result) {
        const row = document.createElement("tr");

        row.innerHTML = `
            <td>${escapeHtml(result.process_id)}</td>
            <td>${result.arrival_time}</td>
            <td>${result.burst_time}</td>
            <td>${result.priority}</td>
            <td>${result.completion_time}</td>
            <td>${result.turnaround_time}</td>
            <td>${result.waiting_time}</td>
            <td>${result.response_time}</td>
        `;
        table.appendChild(row);

    });
    // Gantt chart
    drawGantt(data.gantt);

}

// Gantt chart
function drawGantt(segments) {

    const container = $("gantt");
    container.innerHTML = "";


    if (!segments ||
        segments.length === 0) {
        container.innerHTML = "<p>No Gantt data available.</p>";
        return;
    }


    segments.forEach(function (segment) {
        const block = document.createElement("div");

        block.className = "gantt-block";

        if (segment.process_id === "IDLE") {
            block.classList.add("idle");
        }

        const duration = segment.end_time - segment.start_time;

        block.style.width =
            Math.max(
                70,
                duration * 45
            ) + "px";

        block.textContent = segment.process_id;
        block.title = segment.process_id + ": " + segment.start_time + " → " + segment.end_time;
        const start = document.createElement("span");
        start.className = "time start";
        start.textContent = segment.start_time;
        const end = document.createElement("span");
        end.className = "time end";
        end.textContent = segment.end_time;
        block.appendChild(start);
        block.appendChild(end);
        container.appendChild(block);
    });
}

// Escape HTML
function escapeHtml(value) {
    return String(value)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

// Run button
$("runButton").addEventListener("click", runAlgorithm);

// Initial sample data
addProcess("P1", 0, 5, 2);
addProcess("P2", 1, 3, 1);
addProcess("P3", 2, 8, 4);
addProcess("P4", 3, 6, 2);

$("quantumField") .classList .add("hidden");