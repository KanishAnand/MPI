#include <bits/stdc++.h>

#include <fstream>

#include "mpi.h"
using namespace std;
typedef long long int ll;

int partition(vector<int> &arr, int beg, int end) {
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

void quick_sort(vector<int> &arr, int beg, int end) {
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

    // taking input and output file path as argument
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
        vector<int> arr;

        vector<int> tmp;

        int a;
        //input
        for (int i = 0; i < n; i++) {
            fin >> a;
            arr.push_back(a);
            tmp.push_back(arr[i]);
        }

        int start = 0, no_per_process = n / numprocs;

        // sending length of subarray to each process
        MPI_Bcast(&no_per_process, 1, MPI_INT, root_process, MPI_COMM_WORLD);

        // sending subarray
        for (int rec_id = 1; rec_id < numprocs; rec_id++) {
            MPI_Send(&arr[start], no_per_process, MPI_INT, rec_id, 0, MPI_COMM_WORLD);
            start += no_per_process;
        }

        // sorting remaining in root process
        quick_sort(arr, start, n - 1);

        vector<vector<int>> sorted_parts(numprocs);

        // storing all sorted subarrays to finally do merge step
        sorted_parts[0] = {arr.begin() + start, arr.end()};

        // recieve back sorted subarray
        for (int rec_id = 1; rec_id < numprocs; rec_id++) {
            vector<int> arr2(no_per_process);
            MPI_Recv(&arr2[0], no_per_process, MPI_INT, rec_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sorted_parts[rec_id] = arr2;
        }

        vector<int> ind(numprocs, 0), ans;
        set<pair<int, int>> st;

        // merging all sorted subarrays
        for (int i = 0; i < numprocs; i++) {
            if (sorted_parts[i].size() != 0) {
                st.insert({sorted_parts[i][0], i});
            }
        }

        for (int i = 0; i < n; i++) {
            auto it = st.begin();
            pair<int, int> pr = *it;
            st.erase(it);
            ans.push_back(pr.first);
            ind[pr.second]++;
            if (ind[pr.second] != sorted_parts[pr.second].size()) {
                st.insert({sorted_parts[pr.second][ind[pr.second]], pr.second});
            }
        }

        sort(tmp.begin(), tmp.end());

        if (tmp == ans) {
            cout << "YES" << endl;
        } else {
            cout << "NO" << endl;
        }

        // output
        ofstream fout(output_file_path);
        for (int i = 0; i < n; i++) {
            fout << ans[i] << " ";
        }

    } else {
        // recieving length of subarray
        int no;
        MPI_Bcast(&no, 1, MPI_INT, root_process, MPI_COMM_WORLD);

        // recieving subarray
        vector<int> arr2(no);
        MPI_Recv(&arr2[0], no, MPI_INT, root_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // sorting subarray
        quick_sort(arr2, 0, no - 1);

        // sending back sorted subarray
        MPI_Send(&arr2[0], no, MPI_INT, root_process, 0, MPI_COMM_WORLD);
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
