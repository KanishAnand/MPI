# Parallel Quick Sort

## Description:

A parallel approach using MPI is followed to sort given array using quick sort algorithm. Initially input array is divided into equal continous subarrays with size of each
subarray depending upon total number of processes. Subarrays are passed to each process where each process sorts their recieved subarrays parallely using quick sort
algorithm. Now after each process sorts its subarray they send back the sorted subarray to root process. Root process also sorts its own part of the subarray.
After recieving all sorted subarray all of them are merged using `Multiset` as explained below.

#### Merge Step:

Multiset contains smallest element of each sorted subarray. Then smallest element of these is pushed to merged sorted array and next smallest element from sorted
subarray to which this smallest element belongs is inserted to multiset. The same process is followed till we built our full merged sorted array.

## Analysis

# Parallel Edge Coloring

## Description:

A parallel approach is to be followed to color the edges of given graph in such a way that no two adjacent edges have same color and maximum number of colors used is `1 + max(Delta of the original graph, Delta of the line graph)`. Initially given graph is transformed to its line graph. The line graph of an undirected graph G is another graph L(G) that represents the adjacencies between edges of G. L(G) is constructed in the following way: for each edge in G, make a vertex in L(G); for every two edges in G that have a vertex in common, make an edge between their corresponding vertices in L(G). Now for this line graph our problem is transformed to graph coloring (vertex-coloring). For this a parallel approach has been used using **Jones-Plassmann** algorithm as explained [here](https://ireneli.eu/2015/10/26/parallel-graph-coloring-algorithms/).

#### Implementation Steps:

- Initially random numbers were assigned to each node.
- Now each process is distributed nodes of the original graph equally in other words we divided our graph into subgraphs and passed the subgraph nodes to each process.
- Each process has task to color its own subgraph but we need to do that parallely so that there are no color conflicts. For this we do this task in iterations.
- In each iteration first find an independent set of subgraph from nodes which are not colored. Process of finding this independent set is that for all nodes which are not colored choose the ones having node value more than all its uncolored neighbours. This way we make an independent set.
- Now as it is independent set we can color all nodes of independent set parallely in each process iteration wise. Color each node of independent set with smallest color not assigned to any of the neighbours this way we ensure that max number of colors used is not more than 1 + Delta of line graph.
- At the end of each iteration send back colored nodes from each process back to root process. In the root process update color array and then broadcast it to all process for next iteration.
- When a process finishes coloring its all nodes stop that process and mark it as done so that root process not waits for any communication with this process now.

## Analysis