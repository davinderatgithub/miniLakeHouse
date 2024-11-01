Project Progess Overview: (Completed)

Implement a distributed query processing engine
Process structured data (student rankings table)
Order student IDs based on batch_year, university_ranking, and batch_ranking
Components to Implement:

Driver Program: Breaks down the problem into smaller tasks and distributes them to Engine instances
Engine Instances: Execute tasks and return results to the Driver

a. Strategy Development: The Driver needs to come up with a strategy to break down the problem into smaller, manageable tasks.
b. Task Distribution: It distributes these smaller tasks to the available Engine instances for processing.
c. Result Aggregation: After the Engine instances process their tasks, the Driver collects and aggregates the results.

Usefull tools: Thread pool

Data Structure:

Table Name: student_rankings
Columns: student_id (varchar), batch_year (int), university_ranking (int), batch_ranking (int)
Data is provided as multiple CSV files without headers

First phase.
1. Load data
2. Sort it based on the orderby clause
3. Project the output to output.txt

second phase
1. create driver and engine process needs port for engine
    - perform the tasks in phase 1

Thrid phase
1. utilize parallel processing
    - use k-way merge to merge chunks from different engines

    Modify the Driver:

    a. Accept multiple engine port numbers as command-line arguments.
    b. Implement a connection manager to handle multiple engine connections.
    c. Develop a task distribution strategy to divide work among engines.
    d. Implement a result aggregation mechanism to combine results from all engines.

    Update the Engine:

    a. Ensure the engine can handle partial datasets.
    b. Implement efficient local sorting and processing.

    Implement Parallelization Strategy:

    a. File-based partitioning: Assign different files to different engines.
    b. Data-based partitioning: Split large files and distribute chunks to engines.

    Update Shell Scripts:

    a. Modify start_engine.sh to launch multiple engine instances.
    b. Update start_driver.sh to pass multiple engine port numbers.