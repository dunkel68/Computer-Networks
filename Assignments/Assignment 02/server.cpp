#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <ctime>
#include <algorithm>
#include <cctype>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

const int TCP_PORT = 8888;
const int UDP_PORT = 8889;
const int MAX_BUFFER_SIZE = 1024;
const int THREAD_POOL_SIZE = 5;
const string DATA_FILE = "records.csv";

// for file access synchronization
mutex file_mutex;

// Thread pool management
atomic<bool> server_running(true);

struct StudentRecord {
    string studentId;
    string name;
    string course;
    int assignmentMarks;
    int quizMarks;
    int sessionalMarks;
    int finalMarks;
    string remarks;
    string teacherId;

    string toString() const {
        stringstream ss;
        ss << studentId << "," << name << "," << course << "," << assignmentMarks << "," << quizMarks << "," 
           << sessionalMarks << "," << finalMarks << "," << remarks << "," << teacherId;
        return ss.str();
    }

    static StudentRecord fromString(const string& str) {
        StudentRecord record;
        stringstream ss(str);
        string token;

        getline(ss, record.studentId, ',');
        getline(ss, record.name, ',');
        getline(ss, record.course, ',');
        
        getline(ss, token, ',');
        record.assignmentMarks = stoi(token);
        
        getline(ss, token, ',');
        record.quizMarks = stoi(token);
        
        getline(ss, token, ',');
        record.sessionalMarks = stoi(token);
        
        getline(ss, token, ',');
        record.finalMarks = stoi(token);
        
        getline(ss, record.remarks, ',');
        getline(ss, record.teacherId, ',');
        
        return record;
    }
};

struct Teacher {
    string teacherId;
    string password;
};

map<string, StudentRecord> studentRecords;
map<string, Teacher> teachers;

void initializeDatabase();
void saveDatabase();
bool authenticateTeacher(const string& teacherId, const string& password);
void handleTcpClient(SOCKET clientSocket);
void handleUdpClient();
string processTeacherRequest(const string& request, const string& teacherId);
string processStudentRequest(const string& request);
void tcpThreadPoolWorker();
inline void trim(string& s);

bool initWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << endl;
        return false;
    }
    return true;
}

void initializeDatabase() {
    lock_guard<mutex> lock(file_mutex);
    
    teachers["T001"] = {"T001", "1111"};
    teachers["T002"] = {"T002", "2222"};
    teachers["T003"] = {"T003", "3333"};
    teachers["T004"] = {"T004", "4444"};
    teachers["T005"] = {"T005", "5555"};
    teachers["T006"] = {"T006", "6666"};
    
    // loading student records from file
    ifstream file(DATA_FILE);
    if (!file.is_open()) {
        cout << "No existing database found. Creating new database." << endl;
        return;
    }
    
    string line;
    while (getline(file, line)) {
        if (!line.empty()) {
            try {
                StudentRecord record = StudentRecord::fromString(line);
                studentRecords[record.studentId] = record;
            } catch (const exception& e) {
                cerr << "Error parsing line: " << line << " - " << e.what() << endl;
            }
        }
    }
    
    file.close();
    cout << "Loaded " << studentRecords.size() << " student records from database." << endl;
}

void saveDatabase() {
    lock_guard<mutex> lock(file_mutex);
    
    ofstream file(DATA_FILE);
    if (!file.is_open()) {
        cerr << "Error: Could not open file for writing: " << DATA_FILE << endl;
        return;
    }
    
    for (const auto& pair : studentRecords) {
        file << pair.second.toString() << endl;
    }
    
    file.close();
    cout << "Database saved with " << studentRecords.size() << " records." << endl;
}

bool authenticateTeacher(const string& teacherId, const string& password) {
    auto it = teachers.find(teacherId);
    if (it != teachers.end() && it->second.password == password) {
        return true;
    }
    return false;
}

void handleTcpClient(SOCKET clientSocket) {
    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived;
    bool authenticated = false;
    string teacherId;
    
    string welcomeMsg = "Welcome to the Teacher Portal. Please authenticate.\nEnter your Teacher ID: ";
    send(clientSocket, welcomeMsg.c_str(), welcomeMsg.length(), 0);
    
    // Authentication
    while (!authenticated) {
        // Receiving teacher details
        memset(buffer, 0, MAX_BUFFER_SIZE);
        bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            closesocket(clientSocket);
            return;
        }
        teacherId = string(buffer, 0, bytesReceived);
        
        string passwordPrompt = "Enter your password: ";
        send(clientSocket, passwordPrompt.c_str(), passwordPrompt.length(), 0);
        
        memset(buffer, 0, MAX_BUFFER_SIZE);
        bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            closesocket(clientSocket);
            return;
        }
        string password = string(buffer, 0, bytesReceived);
        
        authenticated = authenticateTeacher(teacherId, password);
        if (!authenticated) {
            string authFailMsg = "Authentication failed. Please try again.\nEnter your Teacher ID: ";
            send(clientSocket, authFailMsg.c_str(), authFailMsg.length(), 0);
        }
    }
    
    string menuMsg = "Authentication successful. Welcome, Teacher " + teacherId + "!\n\n"
                         "1. Add a Student Record\n"
                         "2. Edit an Existing Record\n"
                         "3. View All My Student Records\n"
                         "4. Save Data to File\n"
                         "5. Exit\n\n"
                         "Enter your choice: ";
    
    send(clientSocket, menuMsg.c_str(), menuMsg.length(), 0);
    
    // interaction loop
    while (true) {
        memset(buffer, 0, MAX_BUFFER_SIZE);
        bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        
        if (bytesReceived <= 0) {
            cout << "Client disconnected." << endl;
            break;
        }
        
        string request(buffer, 0, bytesReceived);
        string response = processTeacherRequest(request, teacherId);
        
        if (response == "EXIT") {
            string exitMsg = "Thank you for using the system. Goodbye!";
            send(clientSocket, exitMsg.c_str(), exitMsg.length(), 0);
            break;
        }
        
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    
    closesocket(clientSocket);
}

string processTeacherRequest(const string& request, const string& teacherId) {
    stringstream ss(request);
    string choice;
    getline(ss, choice);
    
    if (choice == "1") {
        string promptMsg = "Enter student details in the following format:\n(ID,Name,Course,Assignment,Quiz,Sessional,Final,Remarks)\n";
        return promptMsg;
    } 
    else if (choice == "2") {
        string promptMsg = "Enter student ID to edit: ";
        return promptMsg;
    } 
    else if (choice == "3") {
        lock_guard<mutex> lock(file_mutex);
        string response = "Student Records for Teacher " + teacherId + ":\n\n";
        response += "ID,Name,Course,Assignment,Quiz,Sessional,Final,Remarks\n";
        
        bool found = false;
        for (const auto& pair : studentRecords) {
            if (pair.second.teacherId == teacherId) {
                found = true;
                response += pair.second.studentId + "," + pair.second.name + "," + pair.second.course + "," + to_string(pair.second.assignmentMarks) + "," +
                           to_string(pair.second.quizMarks) + "," + to_string(pair.second.sessionalMarks) + "," + to_string(pair.second.finalMarks) + "," + pair.second.remarks + "\n";
            }
        }
        
        if (!found) {
            response += "No records found for your teacher ID.\n";
        }
        
        response += "\nEnter your choice: ";
        return response;
    } 
    else if (choice == "4") {
        saveDatabase();
        return "Database saved successfully.\n\nEnter your choice: ";
    } 
    else if (choice == "5") {
        return "EXIT";
    } 
    else if (choice.find("S") == 0 && request.find(",") != string::npos) {
        try {
            string record = request;
            stringstream recordSS(record);
            string token;
            
            StudentRecord newRecord;
            
            getline(recordSS, newRecord.studentId, ',');
            getline(recordSS, newRecord.name, ',');
            getline(recordSS, newRecord.course, ',');
            
            getline(recordSS, token, ',');
            newRecord.assignmentMarks = stoi(token);
            
            getline(recordSS, token, ',');
            newRecord.quizMarks = stoi(token);
            
            getline(recordSS, token, ',');
            newRecord.sessionalMarks = stoi(token);
            
            getline(recordSS, token, ',');
            newRecord.finalMarks = stoi(token);
            
            getline(recordSS, newRecord.remarks);
            newRecord.teacherId = teacherId;
            
            {
                lock_guard<mutex> lock(file_mutex);
                studentRecords[newRecord.studentId] = newRecord;
            }
            
            saveDatabase();
            
            return "Student record added successfully.\n\nEnter your choice: ";
        } 
        catch (const exception& e) {
            return "Error adding student record: " + string(e.what()) + 
                   "\nPlease try again using the correct format.\n\nEnter your choice: ";
        }
    } 
    else if (request.find("S") == 0 && request.find(",") == string::npos) {
        string studentId = request;
        
        // Checking if student exists and belongs to that specific teacher
        {
            lock_guard<mutex> lock(file_mutex);
            auto it = studentRecords.find(studentId);
            if (it == studentRecords.end()) {
                return "Student ID not found.\n\nEnter your choice: ";
            }
            
            if (it->second.teacherId != teacherId) {
                return "You can only edit records for students assigned to you.\n\nEnter your choice: ";
            }
            
            string currentData = "Current data: " + it->second.toString() + "\n";
            currentData += "Enter updated details in the below format (leave empty to keep current value):\n";
            currentData += "AssignmentMarks,QuizMarks,SessionalMarks,FinalMarks,Remarks\n";
            return currentData;
        }
    } 
    else if (count(request.begin(), request.end(), ',') >= 4) {
        try {
            stringstream recordSS(request);
            string token;
            
            string assignmentMarks, quizMarks, sessionalMarks, finalMarks, remarks;
            
            getline(recordSS, assignmentMarks, ',');
            getline(recordSS, quizMarks, ',');
            getline(recordSS, sessionalMarks, ',');
            getline(recordSS, finalMarks, ',');
            getline(recordSS, remarks);
            
            // Find the student we're editing (we need to retrieve the student ID from the previous request)
            // For this simplified implementation, we'll look for the most recent student accessed by this teacher
            string studentId;
            {
                lock_guard<mutex> lock(file_mutex);
                for (auto& pair : studentRecords) {
                    if (pair.second.teacherId == teacherId) {
                        studentId = pair.first;
                        break;
                    }
                }
                
                if (studentId.empty()) {
                    return "Error: Could not find a student to edit.\n\nEnter your choice: ";
                }
                
                // Updating
                if (!assignmentMarks.empty()) {
                    studentRecords[studentId].assignmentMarks = stoi(assignmentMarks);
                }
                if (!quizMarks.empty()) {
                    studentRecords[studentId].quizMarks = stoi(quizMarks);
                }
                if (!sessionalMarks.empty()) {
                    studentRecords[studentId].sessionalMarks = stoi(sessionalMarks);
                }
                if (!finalMarks.empty()) {
                    studentRecords[studentId].finalMarks = stoi(finalMarks);
                }
                if (!remarks.empty()) {
                    studentRecords[studentId].remarks = remarks;
                }
            }
            
            saveDatabase();
            
            return "Student record updated successfully.\n\nEnter your choice: ";
        } 
        catch (const exception& e) {
            return "Error updating student record: " + string(e.what()) + 
                   "\nPlease try again using the correct format.\n\nEnter your choice: ";
        }
    }
    
    return "Invalid choice or format. Please try again.\n\nEnter your choice: ";
}

string processStudentRequest(const string& request) {
    string studentId = request;
    trim(studentId);
    
    lock_guard<mutex> lock(file_mutex);
    auto it = studentRecords.find(studentId);
    
    if (it == studentRecords.end()) {
        return "Student ID not found.";
    }
    
    StudentRecord record = it->second;
    stringstream ss;
    
    ss << "Student Record\n"
       << "--------------\n"
       << "ID: " << record.studentId << "\n"
       << "Name: " << record.name << "\n"
       << "Course: " << record.course << "\n"
       << "Assignment Marks: " << record.assignmentMarks << "\n"
       << "Quiz Marks: " << record.quizMarks << "\n"
       << "Sessional Marks: " << record.sessionalMarks << "\n"
       << "Final Marks: " << record.finalMarks << "\n"
       << "Total Marks: " << (record.assignmentMarks + record.quizMarks + 
                              record.sessionalMarks + record.finalMarks) << "\n"
       << "Remarks: " << record.remarks;
    
    return ss.str();
}

// function to trim whitespace
inline void trim(string& s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
}

// TCP thread pool worker
void tcpThreadPoolWorker() {
    // Creating TCP socket for listening
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "Error creating TCP socket: " << WSAGetLastError() << endl;
        return;
    }
    
    // TCP socket address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(TCP_PORT);
    
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "TCP bind failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        return;
    }
    
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "TCP listen failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        return;
    }
    
    cout << "TCP server started on port " << TCP_PORT << endl;
    
    vector<thread> workerThreads;
    
    while (server_running) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        
        if (clientSocket == INVALID_SOCKET) {
            if (server_running) {
                cerr << "TCP accept failed: " << WSAGetLastError() << endl;
            }
            continue;
        }
        
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        cout << "TCP client connected: " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
        
        // Handle client in a new thread
        workerThreads.push_back(thread(handleTcpClient, clientSocket));
    }
    
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    closesocket(listenSocket);
}

void handleUdpClient() {
    // Creating UDP socket
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) {
        cerr << "Error creating UDP socket: " << WSAGetLastError() << endl;
        return;
    }
    
    // UDP socket address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(UDP_PORT);
    
    if (bind(udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "UDP bind failed: " << WSAGetLastError() << endl;
        closesocket(udpSocket);
        return;
    }
    
    cout << "UDP server started on port " << UDP_PORT << endl;
    
    char buffer[MAX_BUFFER_SIZE];
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    
    // Set socket to non-blocking mode
    u_long mode = 1;  // 1 for non-blocking, 0 for blocking
    ioctlsocket(udpSocket, FIONBIO, &mode);
    
    while (server_running) {
        memset(buffer, 0, MAX_BUFFER_SIZE);
        
        // Receive UDP datagram
        int bytesReceived = recvfrom(udpSocket, buffer, MAX_BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
        
        if (bytesReceived == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                // No data available, sleep a bit to avoid busy waiting
                this_thread::sleep_for(chrono::milliseconds(10));
                continue;
            } else {
                cerr << "UDP receive error: " << error << endl;
                continue;
            }
        }
        
        if (bytesReceived > 0) {
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            cout << "UDP request from " << clientIP << ":" << ntohs(clientAddr.sin_port) << " - Student ID: " << buffer << endl;
            
            // Processing request and send response
            string request(buffer, 0, bytesReceived);
            string response = processStudentRequest(request);
            
            sendto(udpSocket, response.c_str(), response.length(), 0, 
                  (sockaddr*)&clientAddr, clientAddrSize);
        }
    }
    
    closesocket(udpSocket);
}

int main() {
    if (!initWinsock()) {
        return 1;
    }
    
    cout << "Starting Teacher-Student Record Management Server..." << endl;
    
    initializeDatabase();
    
    thread udpThread(handleUdpClient);
    
    tcpThreadPoolWorker();
    
    server_running = false;
    if (udpThread.joinable()) {
        udpThread.join();
    }
    
    WSACleanup();
    cout << "Server shutdown complete." << endl;
    
    return 0;
}