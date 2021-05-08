# MPI

Few algorithms implemented in distributed manner in C++ using [MPI](https://www.open-mpi.org/).

# Run

- Compile using: `mpic++ filename`
- Execute using: `mpirun -np <no of processes> ./a.out <input file path> <output file path>`

## Parallel Sum

### Description:

A parallel approach using MPI is followed to find sum of the reciprocals of the squares of integer from 1 to N. Integers from 1 to N are divided equally to each process in form of pair of staring, ending point. Each process finds sum of reciprocal of sqaures of integers from this starting to ending point and pass this sum back to root process. Finally in root process value computed in this process and returned from each process is totalled to get the final output.

### Analysis

For N = 10000, Time Taken (in seconds):

- 1 Process: 0.0002481
- 4 Processes: 0.0002283
- 11 Processes: 0.0002231

Time will increase with increase in value of N almost monotonically due to increased number of operations.

## Parallel Quick Sort

### Description:

A parallel approach using MPI is followed to sort given array using quick sort algorithm. Initially input array is divided into equal continous subarrays with size of each
subarray depending upon total number of processes. Subarrays are passed to each process where each process sorts their recieved subarrays parallely using randomized quick sort
algorithm. Now after each process sorts its subarray they send back the sorted subarray to root process. Root process also sorts its own part of the subarray.
After recieving all sorted subarray all of them are merged using `C++ STL Set` as explained below.

#### Merge Step:

Set contains smallest element of each sorted subarray. Then smallest element of these is pushed to merged sorted array and next smallest element from sorted
subarray to which this smallest element belongs is inserted to set. The same process is followed till we build full merged sorted array.

### Analysis

For N = 1e6 with random values of array from 0 till 1e9, Time Taken (in seconds):

- 1 Process: 0.718
- 4 Processes: 0.7634
- 11 Processes: 0.869

Time will increase with increase in value of N almost monotonically due to increased number of operations.  
It is a bit surprising thing to note that instead of decreasing time is increasing with increase in number of processes but it can be justified as overhead of sending and recieving from processes is more than time taken to compute very simple quick sort.

## Parallel Edge Coloring

### Description:

A parallel approach is to be followed to color the edges of given graph in such a way that no two adjacent edges have same color and maximum number of colors used is not more than `1 + max(Delta of the original graph, Delta of the line graph)`. Initially given graph is transformed to its line graph. The line graph of an undirected graph G is another graph L(G) that represents the adjacencies between edges of G. L(G) is constructed in the following way: for each edge in G, make a vertex in L(G); for every two edges in G that have a vertex in common, make an edge between their corresponding vertices in L(G). Now for this line graph our problem is transformed to graph coloring (vertex-coloring). For this a parallel approach has been followed using **Jones-Plassmann** algorithm as explained [here](https://ireneli.eu/2015/10/26/parallel-graph-coloring-algorithms/).

##### Implementation Steps:

- Initially assign random numbers to each node.
- Now distribute nodes of original graph to each process equally or in other words divide original graph into subgraphs and pass the subgraph nodes to each process.
- Each process has task to color its own subgraph but we need to do that parallely so that there are no color conflicts. For this we do this task in iterations.
- In each iteration first find an independent set of subgraph from nodes which are not colored. Process of finding this independent set is that for all nodes which are not colored choose the ones having node value more than all its uncolored neighbours. This way we make an independent set.
- Now as it is an independent set we can color all nodes of independent set parallely in each process iteration wise. Color each node of independent set with smallest color not assigned to any of the neighbours this way we ensure that max number of colors used is not more than 1 + Delta of line graph.
- At the end of each iteration send back colored nodes from each process to root process. In the root process update color array and then broadcast it to all processes for next iteration.
- When a process finishes coloring all its nodes stop that process and mark it as done so that root process not waits for any communication with this process now.

### Analysis

For 128 vertices and 774 edges(random), Time Taken (in seconds):

- 1 Process: 0.03832
- 4 Processes: 0.04283
- 11 Processes: 0.037641
  Time will increase with increase in value of N almost monotonically due to increased number of operations.
