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

void find_independentSet(int start, int end, vector<int> &node_val,
                         vector<int> &color, vector<vector<int>> &LG_adj,
                         vector<int> &independent_set, int &done) {
    for (int i = start; i <= end; i++) {
        if (color[i] != -1) {
            continue;
        }

        int flag = 1;
        for (auto neigh : LG_adj[i]) {
            if (color[neigh] == -1 && node_val[neigh] > node_val[i]) {
                done = 0;
                flag = 0;
                break;
            }
        }
        if (flag == 1) {
            independent_set.push_back(i);
        }
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

    int LG_n, LG_m;
    vector<int> edge_first, edge_second, node_val;

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

        // initialising node values with random numbers
        vector<int> node_val(LG_n + 1);
        vector<int> tmp;
        for (int i = 1; i <= LG_n; i++) {
            tmp.push_back(i);
        }
        random_shuffle(tmp.begin(), tmp.end());
        for (int i = 1; i <= LG_n; i++) {
            node_val[i] = tmp[i - 1];
        }

        // broadcasting line graph
        LG_m = edge_first.size();
        MPI_Bcast(&LG_n, 1, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&LG_m, 1, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&edge_first[0], LG_m, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&edge_second[0], LG_m, MPI_INT, root_process, MPI_COMM_WORLD);
        MPI_Bcast(&node_val[0], LG_n + 1, MPI_INT, root_process, MPI_COMM_WORLD);

        vector<int> color(LG_n + 1, -1);
        int start = 1, end, no_per_process = LG_n / numprocs;

        // distributing nodes all processes
        for (int rec_id = 1; rec_id < numprocs; rec_id++) {
            end = start + no_per_process - 1;
            pair<int, int> pr = {start, end};
            MPI_Send(&pr, 2, MPI_INT, rec_id, 0, MPI_COMM_WORLD);
            start = end + 1;
        }

        vector<int> finished(numprocs + 1, 0);

        while (1) {
            int alldone = 1;

            //check if all done or not
            for (int i = 1; i <= LG_n; i++) {
                if (color[i] != -1) {
                    continue;
                }
                alldone = 0;
            }

            if (alldone == 1) {
                break;
            }

            // sending upodated colors to each process for new iteration
            // MPI_Bcast(&color[0], LG_n + 1, MPI_INT, root_process, MPI_COMM_WORLD);
            for (int rec_id = 1; rec_id < numprocs; rec_id++) {
                if (finished[rec_id] == 1) {
                    continue;
                }
                MPI_Send(&color[0], LG_n + 1, MPI_INT, rec_id, 0, MPI_COMM_WORLD);
            }

            // recieving back colored nodes
            for (int rec_id = 1; rec_id < numprocs; rec_id++) {
                if (finished[rec_id] == 1) {
                    continue;
                }

                int done;
                vector<pair<int, int>> changed_colors(no_per_process);
                MPI_Recv(&done, 1, MPI_INT, rec_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&changed_colors[0], 2 * (no_per_process), MPI_INT, rec_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                finished[rec_id] = done;

                // updating colors
                for (auto pr : changed_colors) {
                    color[pr.first] = pr.second;
                }
            }

            // for remaining do in root process
            int tmp_done = 0;
            vector<int> independent_set;
            find_independentSet(start, LG_n, node_val, color, LG_adj, independent_set, tmp_done);
            vector<pair<int, int>> tmp;
            color_it(independent_set, LG_adj, color, tmp);
        }

        //output answer
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

        node_val.resize(LG_n + 1);
        MPI_Bcast(&node_val[0], LG_n + 1, MPI_INT, root_process, MPI_COMM_WORLD);

        //making adjacency list
        vector<vector<int>> LG_adj(LG_n + 2);
        for (int i = 0; i < LG_m; i++) {
            LG_adj[edge_first[i]].push_back(edge_second[i]);
            LG_adj[edge_second[i]].push_back(edge_first[i]);
        }

        // recieving start and end point of nodes of this process
        pair<int, int> pr;
        MPI_Recv(&pr, 2, MPI_INT, root_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int start = pr.first, end = pr.second;

        while (1) {
            // recieving colors
            vector<int> color(LG_n + 1);
            // MPI_Bcast(&color[0], LG_n + 1, MPI_INT, root_process, MPI_COMM_WORLD);
            MPI_Recv(&color[0], LG_n + 1, MPI_INT, root_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // find independent set
            int done = 1;
            vector<int> independent_set;
            find_independentSet(start, end, node_val, color, LG_adj, independent_set, done);

            //coloring this independent set of nodes
            vector<pair<int, int>> changed_colors;
            color_it(independent_set, LG_adj, color, changed_colors);

            // return color array
            MPI_Send(&done, 1, MPI_INT, root_process, 0, MPI_COMM_WORLD);
            MPI_Send(&changed_colors[0], 2 * int(changed_colors.size()), MPI_INT, root_process, 0, MPI_COMM_WORLD);
            if (done == 1) {
                break;
            }
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
