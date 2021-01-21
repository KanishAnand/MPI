#include <bits/stdc++.h>

#include <fstream>

#include "mpi.h"
using namespace std;
typedef long long int ll;

void color_it(vector<int> &independent_set, vector<vector<int>> &LG_adj,
              vector<int> &color, vector<pair<int, int>> &changed_colors) {
    for (auto node : independent_set) {
        set<int> st;
        for (auto neigh : LG_adj[node]) {
            // smallest number which is not any neighbours color
            if (color[neigh] != -1) {
                st.insert(color[neigh]);
            }
        }

        // color node with smallest number which is not any neighbours color
        if (st.size() == 0) {
            color[node] = 1;
        } else {
            int cnt = 1;
            for (auto p : st) {
                if (p != cnt) {
                    color[node] = cnt;
                    break;
                }
                cnt++;
            }
            if (color[node] == -1) {
                color[node] = cnt;
            }
        }

        changed_colors.push_back({node, color[node]});
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

    int rank, numprocs, CONTINUE_TAG = 0, STOP_TAG = 1;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    /*synchronize all processes*/
    MPI_Barrier(MPI_COMM_WORLD);
    double tbeg = MPI_Wtime();
    int root_process = 0, REC = 1;

    int LG_n, LG_m;
    vector<int> edge_first, edge_second;

    /* write your code here */
    if (rank == root_process) {
        int n, m;
        vector<pair<int, int>> edges;

        int a, b;
        // input graph
        ifstream fin(input_file_path);
        fin >> n >> m;
        for (int i = 0; i < m; i++) {
            fin >> a >> b;
            if (a > b) {
                swap(a, b);
            }
            edges.push_back({a, b});
        }

        // make line graph from original graph
        LG_n = m;
        vector<vector<int>> LG_adj(LG_n + 2);

        for (int i = 0; i < m; i++) {
            for (int j = i + 1; j < m; j++) {
                //make edges between line graph vertices if there is some common vertex between two edges in original graph
                if (edges[i].first == edges[j].first || edges[i].first == edges[j].second ||
                    edges[i].second == edges[j].first || edges[i].second == edges[j].second) {
                    LG_adj[i + 1].push_back(j + 1);
                    LG_adj[j + 1].push_back(i + 1);
                    edge_first.push_back(i + 1);
                    edge_second.push_back(j + 1);
                }
            }
        }

        // broadcasting line graph
        LG_m = edge_first.size();
        MPI_Bcast(&LG_n, 1, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&LG_m, 1, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&edge_first[0], LG_m, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&edge_second[0], LG_m, MPI_INT, root_process, MPI_COMM_WORLD);

        // initialising node values with random numbers
        vector<int> node_val(LG_n + 2);
        vector<int> tmp;
        for (int i = 1; i <= LG_n; i++) {
            tmp.push_back(i);
        }
        random_shuffle(tmp.begin(), tmp.end());
        for (int i = 1; i <= LG_n; i++) {
            node_val[i] = tmp[i - 1];
        }

        vector<int> color(LG_n + 1, -1);

        while (1) {
            int alldone = 1;

            //making independent set of nodes
            vector<int> independent_set;
            for (int i = 1; i <= LG_n; i++) {
                if (color[i] != -1) {
                    continue;
                }

                alldone = 0;
                int flag = 1;
                for (auto neigh : LG_adj[i]) {
                    if (color[neigh] == -1 && node_val[neigh] > node_val[i]) {
                        flag = 0;
                        break;
                    }
                }
                if (flag == 1) {
                    independent_set.push_back(i);
                }
            }

            if (alldone == 1) {
                break;
            }

            int start = 0, no_per_process = int(independent_set.size()) / numprocs;

            // distributing independent set to all processes as they can be colored parallely
            for (int rec_id = 1; rec_id < numprocs; rec_id++) {
                MPI_Send(&no_per_process, 1, MPI_INT, rec_id, CONTINUE_TAG, MPI_COMM_WORLD);
                MPI_Send(&independent_set[start], no_per_process, MPI_INT, rec_id, CONTINUE_TAG, MPI_COMM_WORLD);
                MPI_Send(&color[0], LG_n + 1, MPI_INT, rec_id, CONTINUE_TAG, MPI_COMM_WORLD);
                start += no_per_process;
            }

            // recieving back colored nodes
            for (int rec_id = 1; rec_id < numprocs; rec_id++) {
                vector<pair<int, int>> changed_colors(no_per_process);
                MPI_Recv(&changed_colors[0], 2 * (no_per_process), MPI_INT, rec_id, CONTINUE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (auto pr : changed_colors) {
                    color[pr.first] = pr.second;
                }
            }

            // for remaining do in root process
            vector<int> tmp_set;
            for (int i = start; i < independent_set.size(); i++) {
                tmp_set.push_back(independent_set[i]);
            }

            vector<pair<int, int>> tmp;
            color_it(tmp_set, LG_adj, color, tmp);
        }

        // stop all processses
        for (int rec_id = 1; rec_id < numprocs; rec_id++) {
            int a = 1;
            MPI_Send(&a, 1, MPI_INT, rec_id, STOP_TAG, MPI_COMM_WORLD);
        }

        //printing answer
        set<int> st;
        for (int i = 1; i <= LG_n; i++) {
            st.insert(color[i]);
        }

        ofstream fout(output_file_path);
        fout << st.size() << endl;
        for (int i = 1; i <= LG_n; i++) {
            fout << color[i] << " ";
        }
    } else {
        // recieving line graph
        MPI_Bcast(&LG_n, 1, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&LG_m, 1, MPI_INT, root_process, MPI_COMM_WORLD);

        edge_first.resize(LG_m), edge_second.resize(LG_m);
        MPI_Bcast(&edge_first[0], LG_m, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&edge_second[0], LG_m, MPI_INT, root_process, MPI_COMM_WORLD);

        vector<vector<int>> LG_adj(LG_n + 2);
        for (int i = 0; i < LG_m; i++) {
            LG_adj[edge_first[i]].push_back(edge_second[i]);
            LG_adj[edge_second[i]].push_back(edge_first[i]);
        }

        while (1) {
            // recieving number of nodes
            int no;
            MPI_Status STATUS;
            MPI_Recv(&no, 1, MPI_INT, root_process, MPI_ANY_TAG, MPI_COMM_WORLD, &STATUS);

            if (STATUS.MPI_TAG == STOP_TAG) {
                break;
            }

            // recieving independent set of ndoes to be colored
            vector<int> independent_set(no);
            MPI_Recv(&independent_set[0], no, MPI_INT, root_process, CONTINUE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // recieving colors
            vector<int> color(LG_n + 1);
            MPI_Recv(&color[0], LG_n + 1, MPI_INT, root_process, CONTINUE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            //coloring this independent set of nodes
            vector<pair<int, int>> changed_colors;
            color_it(independent_set, LG_adj, color, changed_colors);

            // return color array
            MPI_Send(&changed_colors[0], 2 * int(changed_colors.size()), MPI_INT, root_process, CONTINUE_TAG, MPI_COMM_WORLD);
        }
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
