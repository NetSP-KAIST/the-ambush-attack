/*
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 * 
 * Copyright (C) 2024 Ambush-attack-authors
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "hotstuff/aequitas.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <random>

#define TXF 'F'
#define TXT 'T'
#define RAND_NUM_SEED 9301
#define RAND_SEED_CNT 1000
#define RAND_QUORUM_CNT 100
#define THEMIS_MAX_TX_CNT 1600

using namespace std;

vector<vector<string>> rand_seed_str;
vector<vector<string>> loc_ords;
vector<vector<size_t>> quorum_perm; // Assume adversary's loc_ord is always included
mt19937 gen(RAND_NUM_SEED);


/*
 * Function to check if the first value's index is smaller than the third 
 * value's index in the Hamiltonian path.
 */
template <typename T>
bool is_frontrunnable(const vector<T>& path, T firstValue, T thirdValue) {
    auto firstPos = find(path.begin(), path.end(), firstValue);
    auto thirdPos = find(path.begin(), path.end(), thirdValue);

    if (firstPos == path.end() || thirdPos == path.end()) {
        cerr << "Error: Values not found in the path." << endl;
        return false;
    }
    return firstPos < thirdPos;
}


// Function to get the index of the target in the source vector
int get_index(vector<string>& src, string target) {
    auto it = find(src.begin(), src.end(), target);
    if (it == src.end()) return -1;
    return it - src.begin();
}


// Function to add a transaction to the destination vector
void add_tx(vector<vector<string>>& src_v, vector<vector<string>>& dest_v, string tx) {
    for (vector<string> ordering : src_v) {
        for (int i = 0; i < ordering.size(); i++) {
            vector<string> new_ordering = ordering;
            new_ordering.insert(new_ordering.begin() + i, tx);
            dest_v.push_back(new_ordering);
        }
        vector<string> new_ordering = ordering;
        new_ordering.push_back(tx);
        dest_v.push_back(new_ordering);
    }
}


// Function to generate random quorum
void generate_random_quorum(const int N, int quorum_size) {
    uniform_int_distribution<> dis(1, N - 1);
    for (int i = 0; i < RAND_QUORUM_CNT; i++) {
        vector<size_t> quorum;
        quorum.push_back(0);
        while (quorum.size() < quorum_size) {
            int rand_num = dis(gen);
            if (quorum.end() != find(quorum.begin(), quorum.end(), rand_num)){
                continue;
            }
            quorum.push_back(rand_num);
        }
        quorum_perm.push_back(quorum);
    }
}


// Recursive function to get quorum
void get_quorum(const int N, int M, vector<int>& ind_vec, int index) {
    if (M == 0) {
        vector<size_t> included_loc_ord(ind_vec.begin(), ind_vec.end());
        quorum_perm.push_back(included_loc_ord);
        return;
    }
    for (int i = index; i < N; ++i) {
        ind_vec.push_back(i);
        get_quorum(N, M - 1, ind_vec, i + 1);
        ind_vec.pop_back();
    }
}


// Function to generate random seed
void generate_random_seed(const vector<string>& tx_list) {
    uniform_int_distribution<> dis(1, 100000);

    for (int i = 0; i < RAND_SEED_CNT; i++) {
        rand_seed_str.push_back(vector<string>());
        vector<pair<int, string>> mixer;
        for (int j = 0; j < tx_list.size(); j++) {
            int rand_num = dis(gen);
            mixer.push_back(make_pair(rand_num, tx_list[j]));
        }

        sort(mixer.begin(), mixer.end());

        for (int j = 0; j < mixer.size(); j++) {
            rand_seed_str[i].push_back(mixer[j].second);
        }
    }
}


/*
 * Function to start multi-threading
 * Parameter curr_ind indicates the index of the thread and it also indicates 
 * the index of the ordering of adv_ord. Once this function is given a curr_ind, 
 * it will only work on the adversary's ordering (adv_ord[curr_ind]), and 
 * permute benign_ord.
 */
int run_sequencing(const size_t node_cnt, const size_t f_cnt, const int quorum_ind) {
    printf("%02d : ", quorum_ind);
    const size_t threshold = f_cnt + 1; // Assume gamma is 1.
    size_t cmd_cnt = THEMIS_MAX_TX_CNT;
    std::vector<string> tx_list;
    std::vector<int> occurrence;
    std::vector<std::vector<int>> weighted_edge;
    std::vector<std::vector<int>> edge;
    // ord_in_int is an int array that translates each char ordering represented
    // in int. Adversary is indexed 0, first benign replica is 1
    std::vector<std::vector<int>> ord_in_int;

    for (int i = 0; i < quorum_perm[quorum_ind].size(); i++) {
        int ord_ind = quorum_perm[quorum_ind][i];
        for (int j = 0; j < loc_ords[ord_ind].size(); j++) {
            int tx_index_in_base = get_index(tx_list, loc_ords[ord_ind][j]);
            if (tx_index_in_base == -1) {
                tx_list.push_back(loc_ords[ord_ind][j]);
                occurrence.push_back(1);
            } else {
                occurrence[tx_index_in_base]++;
            }
        }
    }
    cmd_cnt = tx_list.size();
    ord_in_int.resize(node_cnt - f_cnt, std::vector<int>(cmd_cnt, 0));
    weighted_edge.resize(cmd_cnt, std::vector<int>(cmd_cnt, 0));
    edge.resize(cmd_cnt, std::vector<int>(cmd_cnt, 0));

    // Applying replica's loc_ord to weighted edge
    for (int i = 0; i < quorum_perm[quorum_ind].size(); i++) {
        int ord_ind = quorum_perm[quorum_ind][i];
        for (int j = 0; j < loc_ords[ord_ind].size(); j++) {
            ord_in_int[i][j] = get_index(tx_list, loc_ords[ord_ind][j]);
            if (ord_in_int[i][j] == -1) {
                fprintf(stderr, "Expected index, got -1\n");
                return -1;
            }
        }
        for (int j = 0; j < loc_ords[ord_ind].size() - 1; j++) {
            for (int k = j + 1; k < loc_ords[ord_ind].size(); k++) {
                weighted_edge[ord_in_int[i][j]][ord_in_int[i][k]]++;
            }
        }
    }

    // Construct unweighted edge based on weighted_edge
    for (int i = 0; i < cmd_cnt - 1; i++) {
        for (int j = i + 1; j < cmd_cnt; j++) {
            if (weighted_edge[i][j] >= weighted_edge[j][i] 
                && weighted_edge[i][j] >= threshold) 
            {
                edge[i][j] = 1;
            } 
            else if (weighted_edge[j][i] > weighted_edge[i][j] 
                && weighted_edge[j][i] >= threshold) 
            {
                edge[j][i] = 1;
            }
        }
    }

    // Repeat r times. Input every random seed.
    // The previously constructed graph is translated based on the random seed. 
    // Then tie-break is performed.
    int succ_rate = 0;
    for (int i = 0; i < rand_seed_str.size(); i++) {
        vector<string> cmd_content;
        // if base_cmd_in_seed[0] == 3, it means base local ordering's 0th cmd 
        // is located at 3 in the seed.
        int base_cmd_in_seed[cmd_cnt];
        Aequitas::TopologyGraph graph(cmd_cnt+1, 1, cmd_cnt);

        for (auto cmd : rand_seed_str[i]) {
            cmd_content.push_back(cmd);
        }

        for (int j = 0; j < cmd_cnt; j++) {
            int index = get_index(tx_list, rand_seed_str[i][j]);
            if (index == -1) {
                fprintf(stderr, "Very big bug\n");
                return -1;
            }
            base_cmd_in_seed[index] = j;
        }
        for (int j = 0; j < cmd_cnt - 1; j++) {
            for (int k = j + 1; k < cmd_cnt; k++) {
                if (edge[j][k] == 1) {
                    graph.addedge(base_cmd_in_seed[j] + 1, base_cmd_in_seed[k] + 1);
                } else if (edge[k][j] == 1) {
                    graph.addedge(base_cmd_in_seed[k] + 1, base_cmd_in_seed[j] + 1);
                }
            }
        }

        int T_pos = get_index(rand_seed_str[i], "T") + 1;
        int F_pos = get_index(rand_seed_str[i], "F") + 1;

        int TF_dist = 0;
        int TF_cycle_size = 0, cycle_cnt = 0;
        float avg_cycle_size = 0;
        vector<string> final_ordered_cmds = graph.finalize(
            cmd_content,
            TF_cycle_size,
            avg_cycle_size,
            cycle_cnt,
            TF_dist,
            T_pos, F_pos, 
            NULL
        );

        if (is_frontrunnable(final_ordered_cmds, string{TXF}, string{TXT})) {
            succ_rate++;
        }
    }

    cout << "attack success rate: " << succ_rate << " / " 
            << rand_seed_str.size() << "\n";
    return succ_rate;
}


// Function to set global variables
int set_global_var(size_t& node_cnt, size_t& f_cnt, ifstream& loc_ord_file){ 
    string buffer, tx;
    stringstream ss;

    getline(loc_ord_file, buffer);
    if (sscanf(buffer.c_str(), "%ld", &node_cnt) != 1) {
        fprintf(stderr, "Number of nodes is not given in the input file\n");
        return -1;
    }
    f_cnt = (node_cnt - 1) / 4;

    cout << "node cnt: " << node_cnt << ", f cnt: " << f_cnt 
         << ", quorum size: " << node_cnt - f_cnt << "\n";

    for (int i = 0; i < node_cnt; i++) {
        buffer.clear();
        ss.clear();
        loc_ords.push_back(vector<string>());
        getline(loc_ord_file, buffer);
        if (buffer.empty()) {
            fprintf(stderr, "Not enough local orderings are given\n");
            return -1;
        }
        ss << buffer;
        while (getline(ss, tx, ',')) {
            tx.erase(remove(tx.begin(), tx.end(), ' '), tx.end());
            if (!tx.empty()) loc_ords[i].push_back(tx);
        }
    }

    size_t loc_ord_len = loc_ords[0].size();
    for (int i = 0; i < node_cnt; i++) {
        if (get_index(loc_ords[i], string{TXF}) == -1 
                || get_index(loc_ords[i], string{TXT}) == -1) 
        {
            fprintf(stderr, "T or F is missing from the base_ordering\n"
                                    "The input cmds must be capitalized\n");
            return -1;
        }
    }

    return 0;
}


int main(int argc, char* argv[]) {
    size_t node_cnt, f_cnt;
    ifstream loc_ord_file;

    if (argc != 2) {
        fprintf(stderr, "Usage: ./a.out <file_name>\n"
            "e.g., ./a.out loc_ord.csv\n"
            "The file should have the format of the following:\n"
            "<num_nodes>\n<local_orderings>\n"
            "e.g.,:\n3\nF, A, T,\nA, T, F,\nT, F, A,\n");
        return 0;
    }

    loc_ord_file.open(argv[1]);
    if (!loc_ord_file.is_open()) {
        fprintf(stderr, "Failed to open file %s\n", argv[1]);
        return 0;
    }
    if (set_global_var(node_cnt, f_cnt, loc_ord_file) != 0) {
        fprintf(stderr, "Error happened while setting global vars\n");
        return 0;
    }

    vector<int> ind_vec = {0};
    if(node_cnt <= 9)
        get_quorum(node_cnt, node_cnt - f_cnt - 1, ind_vec, 1);
    else generate_random_quorum(node_cnt, node_cnt - f_cnt);

    cout << quorum_perm.size() << " possible quorum permutations exist\n\n";
    int tot_succ_rate = 0;
    uniform_int_distribution<> dis(0, quorum_perm.size() - 1);
    generate_random_seed(loc_ords[0]);

    for (int i = 0; i < RAND_QUORUM_CNT; i++) {
        int r = dis(gen);
        if(i >= quorum_perm.size()) break;
        tot_succ_rate += run_sequencing(node_cnt, f_cnt, i);
    }

    if(quorum_perm.size() > RAND_QUORUM_CNT){
        cout << "\ntotal: " << tot_succ_rate << " / " 
            << RAND_QUORUM_CNT * RAND_SEED_CNT << "\n";
    }
    else{
        cout << "\ntotal: " << tot_succ_rate << " / " 
            << quorum_perm.size() * RAND_SEED_CNT << "\n";
    }

    return 0;
}
