
# Problem 1

  

Use the numerical identity that the sum of the reciprocals of the squares of integers converges to $\pi$<sup>2</sup> / 6.

## Description

The process 0 divides the range [1, n] equally and sends each process a pair {a, b} to calculate the sum of the reciprocals of the squares of integers from a to b. Each process sends the calculated value back to root process. The root process adds all the values and round it off to 6 decimal places.

## Analysis

For an array of 10000 elements, time taken (in sec) data is :
 
 - 1 Process:         0.000258
 - 5 Processes:     0.000215
 - 11 Processes:   0.000208

# Problem 2
  
  Given an array of numbers, your task is to return the array in sorted order by implementing parallel quicksort.


## Description

First the array is equally partitioned by the process 0 among the processes and each process sort the array using randomized quicksort and sent it back to process 0. Then all this np (no. of process) no. of arrays are merged by following algorithm:- 

- Push the minimum element of each array in min heap.
- Take out the minimum element from the heap and add it to sorted array.
- Now insert the next element from the same array in which the removed element belonged.
- Repeat the above 2 steps until min heap is empty.

## Analysis


For an array of 10000 elements, time taken (in sec) data is :
 
 - 1 Process:         0.006
 - 5 Processes:     0.010
 - 11 Processes:   0.014

For an array of 100000 elements, time taken (in sec) data is :
 
 - 1 Process:         0.057
 - 5 Processes:     0.068
 - 11 Processes:   0.179

For an array of 1000000 elements, time taken (in sec) data is :
 
 - 1 Process:         0.72
 - 5 Processes:     0.76
 - 11 Processes:   1.40
  

# Problem 3

Given an undirected graph G, find a proper edge coloring of the graph using 1 + max(Delta(G), Delta(line graph of G)) colors or fewer.
No 2 adjacent edges should have a same color. Delta(G) is the maximum degree of any vertex in G.

## Description

Process 0 broadcasts the edges to all processes and all the processes forms the line graph. Now each edge is a node in our graph. Process 0 gives random color from 1 to Delta to each node and broadcasts the color array.

Process 0 equally partitions the nodes among the processes and all the processes do the following:-

- All the uncolored nodes are selected which have the maximum color value among their uncolored neighbours. These nodes form the independent set.
- All the selected nodes are given the smallest color value not in their neighbour.
- All the process sends the array of selected nodes and their color value to the process 0. Process 0 updates the color array and broadcasts to all the processes.
- Above 2 steps are updated until all the nodes get colored.

## Analysis

For 120 nodes and 1276 edges, time taken (in sec) data is :
 
 - 1 Process:         0.093
 - 5 Processes:     0.052
 - 11 Processes:   0.109

For 128 nodes and 774 edges, time taken (in sec) data is :
 
 - 1 Process:         0.037
 - 5 Processes:     0.022
 - 11 Processes:   0.041

For 36 nodes and 580 edges, time taken (in sec) data is :
 
 - 1 Process:         0.087
 - 5 Processes:     0.042
 - 11 Processes:   0.121
 