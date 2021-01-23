#include <bits/stdc++.h>

#include <fstream>

#include "mpi.h"
using namespace std;
typedef long long int ll;

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
        ifstream fin(input_file_path);
        //input
        int n;
        fin >> n;
        int start = 1, no_per_process = n / numprocs;

        //distribute [1, n] to all processes
        for (int rec_id = 1; rec_id < numprocs; rec_id++) {
            pair<int, int> pr = {start, start + no_per_process};
            MPI_Send(&pr, 2, MPI_INT, rec_id, 0, MPI_COMM_WORLD);
            start += no_per_process;
        }

        double ans = 0;
        // do remaining in root process
        for (int i = start; i <= n; i++) {
            ans += 1.0 / (double)(i * i);
        }

        double sum;
        //recieve sum from each process
        for (int rec_id = 1; rec_id < numprocs; rec_id++) {
            MPI_Recv(&sum, 1, MPI_DOUBLE, rec_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            ans += sum;
        }

        //output
        ofstream fout(output_file_path);
        fout << fixed << setprecision(6) << ans;
    } else {
        pair<int, int> pr;
        MPI_Recv(&pr, 2, MPI_INT, root_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        double sum = 0;
        for (int i = pr.first; i < pr.second; i++) {
            sum += 1.0 / (double)(i * i);
        }

        MPI_Send(&sum, 1, MPI_DOUBLE, root_process, 0, MPI_COMM_WORLD);
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
