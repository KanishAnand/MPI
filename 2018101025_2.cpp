#include <bits/stdc++.h>

#include <fstream>

#include "mpi.h"
using namespace std;
typedef long long int ll;

int partition(int arr[], int beg, int end) {
    // chosing random pivot
    int ind = beg + rand() % (end - beg + 1);
    int pivot = arr[ind];
    swap(arr[ind], arr[end]);

    int i = (beg - 1);
    for (int j = beg; j < end; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }

    int pivot_ind = i + 1;
    swap(arr[end], arr[pivot_ind]);

    return pivot_ind;
}

void quick_sort(int arr[], int beg, int end) {
    if (beg < end) {
        int pivot = partition(arr, beg, end);
        quick_sort(arr, beg, pivot - 1);
        quick_sort(arr, pivot + 1, end);
    }
}

int main(int argc, char **argv) {
    string input_file_path, output_file_path;
    if (argc != 3) {
        cout << "Give 2 arguments in form: ./a.out <path-to-input> <path-to-output>" << endl;
        return 0;
    }

    // taking input and output file path as input
    input_file_path = argv[1];
    output_file_path = argv[2];

    int rank, numprocs;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    /*synchronize all processes*/
    MPI_Barrier(MPI_COMM_WORLD);
    double tbeg = MPI_Wtime();
    int root_process = 0, REC = 1;

    /* write your code here */
    if (rank == root_process) {
        int n;
        ifstream fin(input_file_path);
        fin >> n;
        int arr[n];

        vector<int> tmp;

        for (int i = 0; i < n; i++) {
            fin >> arr[i];
            tmp.push_back(arr[i]);
        }

        int start = 0, no_per_process = n / numprocs;
        for (int rec_id = 1; rec_id < numprocs; rec_id++) {
            // sending length of subarray
            MPI_Send(&no_per_process, 1, MPI_INT, rec_id, 0, MPI_COMM_WORLD);
            // sending subarray
            MPI_Send(arr + start, no_per_process, MPI_INT, rec_id, 0, MPI_COMM_WORLD);
            start += no_per_process;
        }

        quick_sort(arr + start, 0, n - 1 - start);

        vector<vector<int>> sorted_parts(numprocs);

        for (int i = 0; i <= n - 1 - start; i++) {
            sorted_parts[0].push_back(arr[start + i]);
        }

        for (int rec_id = 1; rec_id < numprocs; rec_id++) {
            int arr2[no_per_process];
            // recieve back sorted subarray
            MPI_Recv(arr2, no_per_process, MPI_INT, rec_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < no_per_process; i++) {
                sorted_parts[rec_id].push_back(arr2[i]);
            }
        }

        vector<int> ans, ind(numprocs + 2, 0);

        // merging all sorted subarrays
        for (int i = 0; i < n; i++) {
            int flag = 0, p;
            for (int j = 0; j < numprocs; j++) {
                if (ind[j] != sorted_parts[j].size()) {
                    if (flag == 0) {
                        p = sorted_parts[j][ind[j]];
                        flag = 1;
                    }
                    p = min(p, sorted_parts[j][ind[j]]);
                }
            }

            for (int j = 0; j < numprocs; j++) {
                if (ind[j] != sorted_parts[j].size()) {
                    if (sorted_parts[j][ind[j]] == p) {
                        ind[j]++;
                        break;
                    }
                }
            }
            ans.push_back(p);
        }

        sort(tmp.begin(), tmp.end());

        if (tmp == ans) {
            cout << "YES" << endl;
        } else {
            cout << "NO" << endl;
        }

        ofstream fout(output_file_path);
        for (int i = 0; i < n; i++) {
            fout << ans[i] << " ";
        }

    } else {
        int no;
        // recieving length of subarray
        MPI_Recv(&no, 1, MPI_INT, root_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int arr2[no];
        // recieving subarray
        MPI_Recv(arr2, no, MPI_INT, root_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // sorting subarray
        quick_sort(arr2, 0, no - 1);
        // sending back sorted subarray
        MPI_Send(arr2, no, MPI_INT, root_process, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double elapsedTime = MPI_Wtime() - tbeg;
    double maxTime;
    MPI_Reduce(&elapsedTime, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        printf("Total time (s): %f\n", maxTime);
    }

    /* shut down MPI */
    MPI_Finalize();
    return 0;
}
