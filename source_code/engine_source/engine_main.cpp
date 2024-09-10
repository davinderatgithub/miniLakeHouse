#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>  // For AF_INET, socket(), connect()
#include <netinet/in.h>  // For struct sockaddr_in
#include <arpa/inet.h>   // For inet_addr()
#include <unistd.h>      // For close()
#include <nlohmann/json.hpp>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <fstream> // file read
#include <filesystem>
#include <sstream> 
#include "../include/student_record.h"

using namespace std;
using json = nlohmann::json;

class Engine {
private:
    int port;
    int server_fd;
    vector<StudentRecord> data;

public:
    // start server
    Engine(int port) : port(port) {
        setup_server();
    }

    void setup_server() {
        int opt = 1;
        
        // Create a socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            throw runtime_error("Failed to create socket");
        }
        
        // allowing address reuse
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        // address structure
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Bind the socket and address
        int bind_result = ::bind(server_fd, (struct sockaddr*)&address, sizeof(address));
        if (bind_result < 0) {
            throw runtime_error("Failed to bind to port " + to_string(port) + ": " + strerror(errno));
        }

        // Listen
        if (listen(server_fd, 3) < 0) {
            throw runtime_error("Failed to listen on socket");
        }
    }

    void start() {
        sockaddr_in address;
        int addrlen = sizeof(address);
        int new_socket = accept(server_fd,
                                (struct sockaddr *)&address,
                                (socklen_t*)&addrlen);

        if (new_socket < 0) {
            runtime_error("Failed to accept connection.");
        }

        char buffer[4096] = {0};
        while (true)
        {
            int valread = read(new_socket, buffer, 4096);
            if (valread == 0) {
                cout << "Client disconnected" << endl;
                break;
            }
            if (valread < 0) {
                cerr << "Error reading from socket " << strerror(errno) << endl;
                break;
            }

            cout << "Task recieved." << endl;
    
            json task = json::parse(buffer);

            json result = process_task(task);

            // Add newline as a message separator
            string result_str = result.dump() + "\n";

            //cout << result_str << endl;
            ssize_t bytes_sent = send(new_socket,
                                      result_str.c_str(),
                                      result_str.length(), 0);

            if (bytes_sent < 0) {
                cerr << "Error sending data " << strerror(errno) << endl;
            }
            else
                cout << "sent data bytes: " << bytes_sent << endl;

            close(new_socket);
        } 
    }

    void read_data(string file_path) {
        ifstream file(file_path);
        //vector<StudentRecord> data;

        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + file_path);
        }

        string line;
        while (getline(file, line)) {
            istringstream iss(line);
            StudentRecord record;
            string field;

            getline(iss, record.student_id, ',');
            getline(iss, field, ',');
            record.batch_year = stoi(field);
            getline(iss, field, ',');
            record.university_ranking = stoi(field);
            getline(iss, field, ',');
            record.batch_ranking = stoi(field);

            data.push_back(record);
        }
    }
    void sort_data() {
        sort(data.begin(), data.end(),
            [](const StudentRecord& a, const StudentRecord& b) {
                if (a.batch_year != b.batch_year)
                    return a.batch_year < b.batch_year;
                if (a.university_ranking != b.university_ranking)
                    return a.university_ranking < b.university_ranking;
                
                return a.batch_ranking < b.batch_ranking;
            });
    }

    void quick_sort(int low, int high) {
        if (low < high) {
            // Partition the array
            int pivot = partition(low, high);

            // Recursively sort the sub-arrays
            quick_sort(low, pivot - 1);
            quick_sort(pivot + 1, high);
        }
    }

    int partition(int low, int high) {
        StudentRecord pivot = data[high];
        int i = low - 1;

        for (int j = low; j < high; j++) {
            if (compare_student_records(data[j], pivot)) {
                i++;
                swap(data[i], data[j]);
            }
        }
        swap(data[i + 1], data[high]);
        return i + 1;
    }

    bool compare_student_records(const StudentRecord& a, const StudentRecord& b) {
        if (a.batch_year != b.batch_year)
            return a.batch_year < b.batch_year;
        if (a.university_ranking != b.university_ranking)
            return a.university_ranking < b.university_ranking;

        return a.batch_ranking < b.batch_ranking;
    }

    // Function to convert StudentRecord to json
    json student_record_to_json(const StudentRecord& record) {
        return json{
            {"student_id", record.student_id},
            {"batch_year", record.batch_year},
            {"university_ranking", record.university_ranking},
            {"batch_ranking", record.batch_ranking}
        };
    }
    // Function to create the JSON result
    json create_result_json() {
        json result = {
            {"status", "task_completed"},
            {"data", json::array()},
            {"chunks", 1},
            {"total_chunks", 1}
        };

        // Serialize each StudentRecord into the data array
        for (const auto& record : data) {
            result["data"].push_back(student_record_to_json(record));
        }

        return result;
    }

    json process_task(const json& task) {
        // Process files and return results
        if (task["action"] != "process_files")
            throw runtime_error("Undefined action expected on engine : " + task["action"].dump());

        vector<string> files_to_read = task["files"].get<vector<string>>();    
        vector<string> processed_data;
        //processed_data.push_back("Files recieved from driver to process at engine: ");

        for (string file : files_to_read) {
            cout << file << endl;
            read_data(file);
            //sort_data();
            quick_sort(0, data.size() - 1);
            //processed_data.insert(processed_data.end(), data.begin(), data.end());
        }

        json result = create_result_json();

        return result;
    }

    ~Engine() {
        close(server_fd);
    }
};



int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port" << endl;
        return 1;
    }

    int port = stoi(argv[1]);
    Engine engine(port);
    engine.start();

    return 0;
}