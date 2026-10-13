#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "scheduler/scheduler.h"

using namespace std;

const int PORT = 8080;
const int BUFFER_SIZE = 8192;

// ---------- File ----------
string readFile(const string &fileName)
{
    ifstream file(fileName);
    if (!file)
        return "";
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ---------- HTTP ----------
void sendResponse(SOCKET socket, const string &status,
                  const string &type, const string &body)
{
    string response =
        "HTTP/1.1 " + status + "\r\n"
                               "Content-Type: " +
        type + "\r\n"
               "Content-Length: " +
        to_string(body.size()) + "\r\n"
                                 "Access-Control-Allow-Origin: *\r\n"
                                 "Access-Control-Allow-Headers: Content-Type\r\n"
                                 "Connection: close\r\n\r\n" +
        body;

    send(socket, response.c_str(), (int)response.size(), 0);
}

// ---------- Simple JSON ----------
string getJsonString(const string &json, const string &key)
{
    size_t p = json.find("\"" + key + "\"");
    if (p == string::npos)
        return "";

    p = json.find(':', p);
    p = json.find('"', p + 1);

    if (p == string::npos)
        return "";

    size_t end = json.find('"', p + 1);
    return end == string::npos ? "" : json.substr(p + 1, end - p - 1);
}

int getJsonInt(const string &json, const string &key, int defaultValue = 0)
{
    size_t p = json.find("\"" + key + "\"");
    if (p == string::npos)
        return defaultValue;

    p = json.find(':', p);
    if (p == string::npos)
        return defaultValue;

    p++;

    while (p < json.size() &&
           (json[p] == ' ' || json[p] == '\t'))
        p++;

    size_t end = p;

    while (end < json.size() && isdigit(json[end]))
        end++;

    if (p == end)
        return defaultValue;

    return stoi(json.substr(p, end - p));
}

vector<string> getProcessObjects(const string &json)
{
    vector<string> result;

    size_t p = json.find("\"processes\"");
    if (p == string::npos)
        return result;

    size_t start = json.find('[', p);
    size_t end = json.find(']', start);

    if (start == string::npos || end == string::npos)
        return result;

    string data = json.substr(start + 1, end - start - 1);

    p = 0;

    while ((p = data.find('{', p)) != string::npos)
    {
        size_t close = data.find('}', p);
        if (close == string::npos)
            break;

        result.push_back(data.substr(p, close - p + 1));
        p = close + 1;
    }

    return result;
}

// ---------- Result → JSON ----------
string processJson(const Process &p)
{
    return "{\"process_id\":\"" + p.pid +
           "\",\"arrival_time\":" + to_string(p.arrivalTime) +
           ",\"burst_time\":" + to_string(p.burstTime) +
           ",\"priority\":" + to_string(p.priority) +
           ",\"completion_time\":" + to_string(p.completionTime) +
           ",\"turnaround_time\":" + to_string(p.turnaroundTime) +
           ",\"waiting_time\":" + to_string(p.waitingTime) +
           ",\"response_time\":" + to_string(p.responseTime) + "}";
}

string ganttJson(const GanttSegment &g)
{
    return "{\"process_id\":\"" + g.processId +
           "\",\"start_time\":" + to_string(g.startTime) +
           ",\"end_time\":" + to_string(g.endTime) + "}";
}

string resultJson(const SchedulingResult &r)
{
    string json =
        "{\"ok\":true,\"data\":{\"run\":{"
        "\"algorithm\":\"" +
        r.algorithm +
        "\",\"time_quantum\":" + to_string(r.timeQuantum) +
        ",\"average_waiting_time\":" + to_string(r.avgWaitingTime) +
        ",\"average_turnaround_time\":" + to_string(r.avgTurnaroundTime) +
        ",\"average_response_time\":" + to_string(r.avgResponseTime) +
        ",\"throughput\":" + to_string(r.throughput) +
        ",\"cpu_utilization\":" + to_string(r.cpuUtilization) +
        ",\"context_switches\":" + to_string(r.contextSwitches) +
        "},\"results\":[";

    for (size_t i = 0; i < r.processes.size(); i++)
    {
        if (i)
            json += ",";
        json += processJson(r.processes[i]);
    }

    json += "],\"gantt\":[";

    for (size_t i = 0; i < r.gantt.size(); i++)
    {
        if (i)
            json += ",";
        json += ganttJson(r.gantt[i]);
    }

    return json + "]}}";
}

// ---------- Simulation ----------
void handleSimulation(SOCKET socket, const string &body)
{
    string algorithm = getJsonString(body, "algorithm");

    if (algorithm.empty())
    {
        sendResponse(socket, "400 Bad Request", "application/json",
                     "{\"ok\":false,\"error\":\"Algorithm is required\"}");
        return;
    }

    int quantum = getJsonInt(body, "timeQuantum");

    vector<string> objects = getProcessObjects(body);

    if (objects.empty())
    {
        sendResponse(socket, "400 Bad Request", "application/json",
                     "{\"ok\":false,\"error\":\"At least one process is required\"}");
        return;
    }

    vector<Process> processes;

    for (const string &obj : objects)
    {
        Process p;
        p.pid = getJsonString(obj, "pid");
        p.arrivalTime = getJsonInt(obj, "arrivalTime");
        p.burstTime = getJsonInt(obj, "burstTime");
        p.priority = getJsonInt(obj, "priority", 1);
        processes.push_back(p);
    }

    SchedulingResult result;
    string error;

    if (!runScheduler(algorithm, processes, quantum, result, error))
    {
        sendResponse(socket, "400 Bad Request", "application/json",
                     "{\"ok\":false,\"error\":\"" + error + "\"}");
        return;
    }

    sendResponse(socket, "200 OK", "application/json",
                 resultJson(result));
}

// ---------- Request Handler ----------
void handleRequest(SOCKET socket)
{
    char buffer[BUFFER_SIZE] = {};
    int received = recv(socket, buffer, BUFFER_SIZE - 1, 0);

    if (received <= 0)
        return;

    string request(buffer, received);

    size_t bodyStart = request.find("\r\n\r\n");
    string body = bodyStart == string::npos
                      ? ""
                      : request.substr(bodyStart + 4);

    cout << "\n"
         << request.substr(0, request.find("\r\n")) << endl;

    // Homepage
    if (request.find("GET / ") == 0)
    {
        string html = readFile("frontend/index.html");

        if (html.empty())
        {
            sendResponse(socket, "404 Not Found",
                         "text/plain", "index.html not found");
            return;
        }

        sendResponse(socket, "200 OK", "text/html", html);
        return;
    }

    // JavaScript
    if (request.find("GET /app.js") == 0)
    {
        string js = readFile("frontend/app.js");

        if (js.empty())
        {
            sendResponse(socket, "404 Not Found",
                         "text/plain", "app.js not found");
            return;
        }

        sendResponse(socket, "200 OK",
                     "application/javascript", js);
        return;
    }

    // Status
    if (request.find("GET /api/status") == 0)
    {
        sendResponse(
            socket,
            "200 OK",
            "application/json",
            "{\"project\":\"CPU Scheduling and Performance Analysis System\","
            "\"backend\":\"C++17\","
            "\"database\":\"MySQL 8.0\","
            "\"databaseConnected\":false,"
            "\"algorithmsImplemented\":5,"
            "\"status\":\"Backend connected successfully\"}");
        return;
    }

    // Algorithms
    if (request.find("GET /api/algorithms") == 0)
    {
        sendResponse(
            socket,
            "200 OK",
            "application/json",
            "[\"FCFS\",\"SJF\",\"SRTF\",\"RR\",\"PRIORITY\"]");
        return;
    }

    // Simulation
    if (request.find("POST /api/simulate") == 0)
    {
        if (body.empty())
        {
            sendResponse(socket, "400 Bad Request",
                         "application/json",
                         "{\"ok\":false,\"error\":\"Invalid HTTP request\"}");
            return;
        }

        handleSimulation(socket, body);
        return;
    }

    sendResponse(socket, "404 Not Found",
                 "text/plain", "404 - Route not found");
}

// ---------- Main ----------
int main()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET serverSocket =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverSocket,
             (sockaddr *)&address,
             sizeof(address)) == SOCKET_ERROR)
    {
        cout << "Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "\nCPU Scheduling Backend Server\n";
    cout << "Running at http://localhost:8080\n";
    cout << "GET  /\n";
    cout << "GET  /app.js\n";
    cout << "GET  /api/status\n";
    cout << "GET  /api/algorithms\n";
    cout << "POST /api/simulate\n";
    cout << "\nWaiting for requests...\n";

    while (true)
    {
        SOCKET client = accept(serverSocket, nullptr, nullptr);

        if (client == INVALID_SOCKET)
            continue;

        handleRequest(client);
        closesocket(client);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}