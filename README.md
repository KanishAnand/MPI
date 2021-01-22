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
