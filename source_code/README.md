# miniLakeHouse
This system allows users to execute ORDER BY clauses directly on distributed data files, enabling efficient sorting operations across a distributed environment.


## Key Features

1. **Distributed Processing**: Our system harnesses the power of distributed computing, allowing data to be processed across multiple nodes simultaneously. This approach significantly reduces the time required for large-scale data operations.

2. **Direct ORDER BY Operations**: Users can perform ORDER BY clauses directly on data files without the need for prior data consolidation or preprocessing. This feature streamlines the sorting process and reduces the overall computational overhead.

3. **Scalability**: The system is designed to scale horizontally, accommodating growing data volumes by simply adding more nodes to the distributed network. This ensures that performance remains consistent even as data sizes increase.

4. **Efficiency**: By processing data where it resides, our system minimizes data movement across the network, resulting in faster processing times and reduced bandwidth usage.

5. **Flexible Data File Support**: The system is capable of working with various data file formats, providing users with the versatility to process diverse datasets without the need for format conversion.



# Project directory structure
```
davinder_singh % tree
.
├── Makefile
├── README.md
├── sample_dataset
├── source_code
│   ├── driver_source
│   │   └── driver_main.cpp
│   ├── engine_source
│   │   └── engine_main.cpp
│   └── include
│       └── student_record.h
├── start_driver.sh
└── start_engine.sh

```

# dependency
We are using nlohmann's JSON library for C++, it makes JSON serialization and deserialization easier. Compile the code with C++17 standard.
```
brew install nlohmann-json
```
# Build
Run make, it creates build directory and install build files.
```bash
cd davinder_singh
make
```

```bash
 % ls build/*
build/driver_main       build/engine_main
```

# Usage

1. Start engine instances as follows using different ports
```bash
./start_engine.sh 9001 9002 9003
```
2. Start driver to process the task.
```bash
./start_driver.sh 9001 9002 9003
```
## Note
Engine will load files from the sampe_dataset directory.
Sorted result will be stored in the output.txt file in current directory.

# Working

1. On Driver we distribute the tasks using file-based partitioning i.e. Assign different files to different engines. 

2. On Engine, we recive the file name as message and load the file and then sort it.
After sorting it returns the data as json objects. 

We are using TCP/IP socket based communication.


# Key components

## StudentRecord struct
```CPP
struct StudentRecord {
    std::string student_id;
    int batch_year;
    int university_ranking;
    int batch_ranking;
};
```

## Driver class
```CPP
class Driver {
private:
    vector<int> engine_sockets;
    vector<string> results_g;
    mutex resultsMutex;
    vector<json> results_json;
    vector<StudentRecord> results_data; 
    vector<int> engine_ports;
public:
...
void process_data(const string& data_directory, const string& output_file) 
void connect_to_engines(const vector<int>& ports)
void merge_N_sorted_vectors(vector<vector<StudentRecord>>& result_agg) 
...
```

## Engine class
```CPP
class Engine {
private:
    int port;
    int server_fd;
    vector<StudentRecord> data;

public:
...
void start()
void quick_sort(int low, int high)
int partition(int low, int high) 
...
```