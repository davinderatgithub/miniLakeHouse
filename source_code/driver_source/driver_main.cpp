#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <sys/socket.h>  // For AF_INET, socket(), connect()
#include <arpa/inet.h>   // For inet_addr()
#include <unistd.h>      // For close()
#include <fstream>
#include <netinet/in.h>  // For struct sockaddr_in
#include <nlohmann/json.hpp>
#include <queue>
#include <algorithm>
#include "../include/student_record.h"

using namespace std;
using json = nlohmann::json;

class Driver {
private:
    vector<int> engine_sockets;
    vector<string> results_g;
    mutex resultsMutex;
    vector<json> results_json;
    vector<StudentRecord> results_data; 
    vector<int> engine_ports;

public:
    Driver(const vector<int>& engine_ports) : engine_ports(engine_ports) {
        connect_to_engines(engine_ports);
    }

    void connect_to_engines(const vector<int>& ports) {
        for (int port : ports) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);

            if (sock == -1) {
                cerr << "Failed to create socket for port " << port << endl;
            }

            sockaddr_in engine_addr;
            engine_addr.sin_family = AF_INET;
            engine_addr.sin_port = htons(port);
            engine_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

            if (connect(sock,
                        (struct sockaddr*)&engine_addr,
                        sizeof(engine_addr)) < 0) {
                cerr << "Failed to connect to engine on port " << port << endl;
                close(sock);
                continue;
            }

            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(engine_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
            cout << "Connected to engine at " << ip_str << ":" << ntohs(engine_addr.sin_port) << endl;

            engine_sockets.push_back(sock);
        }
    }

    void send_task(int sock, const json&task) {
        string task_str = task.dump();
        send(sock, task_str.c_str(), task_str.length(), 0);
    }

    void receive_results() {
        vector<thread> threads;
        for (int sock : engine_sockets) {
            threads.emplace_back(&Driver::reveive_results_from_engine, this, sock);
        }
        for (auto& thread : threads) {
            thread.join();
        }
    }

    void reveive_results_from_engine(int sock) {
        string buffer;
        char temp_buffer[1024];
        int bytes_received;

        while (true) {
            bytes_received = recv(sock, temp_buffer, sizeof(temp_buffer) - 1, 0);
            cout << "bytes_received: " << bytes_received << endl;
            if (bytes_received <= 0) {
                break;
            }

            temp_buffer[bytes_received] = '\0';
            buffer += temp_buffer;

            while (!buffer.empty()) {
                // Find the position of the first complete JSON object
                size_t object_end = buffer.find_first_of('\n');
                if (object_end == string::npos) {
                    // No complete object found, wait for more data
                    break;
                }
                object_end++; // Include the closing brace

                // Extract the JSON string
                string json_str = buffer.substr(0, object_end);
                buffer.erase(0, object_end);

                // Remove any leading whitespace or newlines
                json_str.erase(0, json_str.find_first_not_of(" \n\r\t"));

                try {
                    cout << "Json parsing..." << endl;
                    json result = json::parse(json_str);
                    cout << "Json parsing Done!" << endl;
                    lock_guard<mutex> lock(resultsMutex);
                    results_json.push_back(result);

                    if (result["status"] == "task_completed") {
                        return;
                    }
                } catch (const json::exception& e) {
                    cerr << "JSON parsing error: " << e.what() << endl;
                    cerr << "Problematic JSON string: " << json_str << endl;
                }
            }
        }
        cout << "reached at exception unexpectedly " << endl;
        throw runtime_error("Connection closed unexpectedly");
    }

    vector <string> get_csv_files(const string& data_directory) {
        // Check if the directory exists
        if (!filesystem::exists(data_directory)) {
            throw runtime_error("Directory does not exist: " + data_directory);
        }

        // Check if it's actually a directory
        if (!filesystem::is_directory(data_directory)) {
            throw runtime_error("Path is not a directory: " + data_directory);
        }
        vector <string> csv_files;
        for (const auto& entry : filesystem::directory_iterator(data_directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                csv_files.push_back(entry.path().string());
            }
        }
        return csv_files;
    }

    // Deserialize json to record
    vector<StudentRecord> json_to_student_records(json json_data) {
        vector<StudentRecord> records;

        try {
            if (json_data.contains("data") && json_data["data"].is_array()) {
                for (const auto& item : json_data["data"]) {
                    StudentRecord record;
                    record.student_id = item["student_id"];
                    record.batch_year = item["batch_year"];
                    record.university_ranking = item["university_ranking"];
                    record.batch_ranking = item["batch_ranking"];
                    records.push_back(record);
                }
            }

            // You can also check other fields if needed
            if (json_data.contains("status")) {
                cout << "Status: " << json_data["status"] << endl;
            }

        } catch (json::parse_error& e) {
            cerr << "JSON parsing error: " << e.what() << endl;
        } catch (json::type_error& e) {
            cerr << "JSON type error: " << e.what() << endl;
        }

        return records;
    }

    void process_data(const string& data_directory,
                      const string& output_file) {
        cout << "Process data." << endl;
        // Divide tasks and send to engine

        // collect all the .csv file names from the data_dir
        vector<string> files_to_read = get_csv_files(data_directory);

        cout << "files count: " << files_to_read.size() << endl;
    
        // send files to the engine
        int engine_count = engine_sockets.size();
        int files_per_thread = (int)ceil((double)files_to_read.size() / engine_count);
        int i = 0;

        cout << "files per thread : " << files_per_thread << endl;

        for (int sock : engine_sockets) {

            // File-based partitioning: Assign different files to different engines.
            int file_count = 0;
            vector<string> files_list_per_thread;
            while (file_count < files_per_thread && i < files_to_read.size()) {
                files_list_per_thread.push_back(files_to_read[i++]);
                ++file_count;
            }

            cout << "engine_socket: " << sock << endl;
            for (auto file_name : files_list_per_thread)
                cout << file_name << endl;

            cout << endl;

            json task = {
                {"action", "process_files"},
                {"files", files_list_per_thread}
            };

            cout << "Attempting to send tasks ..." << endl;
            send_task(sock, task);
            cout << "Tasks sent to socket: " << sock << endl;
        }

        // Receive and process results
        // Revieve results from all the engine instances
        receive_results();
        cout << "received messages: " << results_json.size() << endl;
        // Deserialize the data for aggregation
        vector<vector<StudentRecord>> result_agg;
        for (const auto& result : results_json) {
            if (result["status"] == "task_completed") {
                cout << "Received result: " << result["status"].dump() << endl;
                auto data = json_to_student_records(result);
                cout << "student records: " << data.size() << endl;
                //results_data.insert(results_data.end(), data.begin(), data.end());
                result_agg.push_back(data);
            }
        }

        //sort(results.begin(), results.end());
        merge_N_sorted_vectors(result_agg);
        write_output(results_data, output_file);
    }
    struct CompareVector {
        bool operator()(const std::pair<StudentRecord, std::pair<int, int>>& a, 
                        const std::pair<StudentRecord, std::pair<int, int>>& b) {
            if (a.first.batch_year != b.first.batch_year)
                return a.first.batch_year < b.first.batch_year;
            if (a.first.university_ranking != b.first.university_ranking)
                return a.first.university_ranking < b.first.university_ranking;
            
            return a.first.batch_ranking < b.first.batch_ranking;
        }
    };

    // <StudentRecord, <vector_number, element_number>>
    void merge_N_sorted_vectors(vector<vector<StudentRecord>>& result_agg) {
        priority_queue<pair<StudentRecord, pair<int, int>>,
                       vector<pair<StudentRecord, pair<int, int>>>,
                       CompareVector> pq;

        // add first element of each vector
        for (int i = 0; i < result_agg.size(); ++i) {
            pq.push({result_agg[0][0], {i, 0}});
        }

        while (!pq.empty()) {
            auto& element = pq.top();

            results_data.push_back(element.first);
            int vector_index = element.second.first;
            int element_index = element.second.second;
            pq.pop();

            // if another element exists in same vector push it into queue
            if (element_index + 1 < result_agg[vector_index].size()) {
                pq.push({result_agg[vector_index][element_index + 1],
                         {vector_index, element_index + 1}});
            }
        }
    }

    void write_output(vector<StudentRecord>& sorted_results, const string& output_file) {
        ofstream out(output_file);
        for (auto record : sorted_results) {
            //cout << record.student_id << endl;
            //out << record.student_id << " " << record.batch_year << " " << record.batch_ranking << " "  << record.university_ranking << endl;
            out << record.student_id << endl;
        }
    }

    ~Driver() {
        for (int sock : engine_sockets)
            close(sock);
    }
};


int main(int argc, char* argv[]) {
    vector<int> engine_ports;
    string data_directory = "./sample_dataset";
    string output_file = "output.txt";

    for (int i = 1; i < argc; ++i) {
        engine_ports.push_back(stoi(argv[i]));
    }

    Driver driver(engine_ports);
    driver.process_data(data_directory, output_file);
    return 0;
}