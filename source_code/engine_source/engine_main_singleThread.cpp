#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <sstream>

using namespace std;

struct StudentRecord {
    string student_id;
    int batch_year;
    int university_ranking;
    int batch_ranking;
};

class SingleThreadDataProcessor {
private:
    string data_directlory;
    vector<StudentRecord> data;

    void read_data() {
        for (const auto& entry : filesystem::directory_iterator(data_directlory)) {
            if (entry.path().extension() == ".csv") {
                ifstream file(entry.path());
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

    void write_output (const string& output_file) {
        ofstream out(output_file);
        for (const auto& record : data) {
            out << record.student_id << "\n";
        }
    }

public:
    SingleThreadDataProcessor(const string& directory) : data_directlory(directory) {}

    void driver_process(string& output_file) {
        read_data();
        sort_data();
        write_output(output_file);
    }
};


int main() {
    string data_directory = "/Users/davindersingh/mywork/miniLakeHouse/source_code/sample_dataset";
    string output_file = "output.txt";

    SingleThreadDataProcessor processor(data_directory);
    processor.driver_process(output_file);

    cout << "Processing complete. Resuls written to " << output_file << endl;
    return 0;
}