# miniLakeHouse
This system allows users to execute ORDER BY clauses directly on distributed data files, enabling efficient sorting operations across a distributed environment.


## Key Features

1. **Distributed Processing**: Our system harnesses the power of distributed computing, allowing data to be processed across multiple nodes simultaneously. This approach significantly reduces the time required for large-scale data operations.

2. **Direct ORDER BY Operations**: Users can perform ORDER BY clauses directly on data files without the need for prior data consolidation or preprocessing. This feature streamlines the sorting process and reduces the overall computational overhead.

3. **Scalability**: The system is designed to scale horizontally, accommodating growing data volumes by simply adding more nodes to the distributed network. This ensures that performance remains consistent even as data sizes increase.

4. **Efficiency**: By processing data where it resides, our system minimizes data movement across the network, resulting in faster processing times and reduced bandwidth usage.

5. **Flexible Data File Support**: The system is capable of working with various data file formats, providing users with the versatility to process diverse datasets without the need for format conversion.


Build

g++ -std=c++17 Driver.cpp -o driver

g++ -std=c++17 Engine.cpp -o engine
