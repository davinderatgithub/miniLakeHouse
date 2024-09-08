#include <iostream>
#include <vector>
#include <string>
#include <sys/socket.h>  // For AF_INET, socket(), connect()
#include <arpa/inet.h>   // For inet_addr()
#include <unistd.h>      // For close()
#include <fstream>
#include <netinet/in.h>  // For struct sockaddr_in
#include <nlohmann/json.hpp>
#include "../include/student_record.h"

using namespace std;
using json = nlohmann::json;

class Driver {
private:
    vector<int> engine_sockets;

public:
    Driver(const vector<int>& engine_ports) {
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

    // json reveive_result(int sock) {
    //     char buffer[4096] = {0};
    //     int valread = read(sock, buffer, 4096);
    //     cout << "read bytes: " << valread << endl;

    //     return json::parse(buffer);
    // }

    vector<json> reveive_results(int sock) {
        vector<json> results;
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
                    results.push_back(result);

                    if (result["status"] == "task_completed") {
                        return results;
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
            // if (json_data.contains("chunks") && json_data.contains("total_chunks")) {
            //     cout << "Received chunk " << j["chunks"] << " of " << j["total_chunks"] << endl;
            // }

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
        int file_counter_inc = ceil(files_to_read.size() / engine_count);
        int i = 0;


        for (int sock : engine_sockets) {

            // file range

            // files_to_read.begin() + i * file_counter_inc, min (files_to_read.begin() + (i + 1) * file_counter_inc,
            //                                                    files_to_read.size()) 
            //vector<string>::iterator range_end = (files_to_read.begin() + (i + 1) * file_counter_inc > files_to_read.size()) ? ;
             json task = {
                 {"action", "process_files"},
                 {"files", files_to_read}
             };
            // json task = {
            //     {"action", "process_files"},
            //     {"files", (files_to_read.begin() + i * file_counter_inc,
            //                min(files_to_read.begin() + (i + 1) * file_counter_inc,
            //                    files_to_read.size()) )} // TODO
           // };

            cout << "Attempting to send tasks ..." << endl;
            send_task(sock, task);
            cout << "Tasks sent!" << endl;
        }

        // Receive and process results
        vector<StudentRecord> results_data;
        for (int sock : engine_sockets) {
            bool task_completed = false;
            while (!task_completed)
            {
                cout << "Waiting to receive results" << endl;
                vector<json> results = reveive_results(sock);
                cout << "received messages: " << results.size() << endl;

                for (const auto& result : results) {
                    if (result["status"] == "task_completed") {
                        cout << "Received result: " << result["status"].dump() << endl;
                   
                        //auto data = result["data"].get<vector<string>>();
                        auto data = json_to_student_records(result);
                        results_data.insert(results_data.end(), data.begin(), data.end());
                        task_completed = true;
                        break;
                    } else if (result["status"] == "EOT") {
                        cout << "All results received" << endl;
                    } else if (result["status"] == "error") {
                        cerr << "Error: " << result["message"] << endl;
                    }
                }
            }
        }

        //Final processing and the output genration
        //sort(results.begin(), results.end());
        write_output(results_data, output_file);
    }
    void write_output(vector<StudentRecord>& sorted_results, const string& output_file) {
        ofstream out(output_file);
        for (auto record : sorted_results) {
            //cout << record.student_id << endl;
            out << record.student_id << endl;
        }
    }

    ~Driver() {
        for (int sock : engine_sockets)
            close(sock);
    }
};

int main() {
    string data_directory = "/Users/davindersingh/mywork/miniLakeHouse/source_code/sample_dataset";
    string output_file = "output.txt";
    vector<int> engine_ports = {9001};  //{9001, 9002, 9003};
    Driver driver(engine_ports);
    driver.process_data(data_directory, output_file);
    return 0;
}