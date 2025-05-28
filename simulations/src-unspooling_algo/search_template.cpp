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
#include <fstream>
#include <sstream>
#include <thread>
#include <future>
#include <chrono>
#include <algorithm>
#include <mutex>

#define TXF 'F'
#define TXT 'T'
#define RESULT_NAME "./data-found_templates/%ldtxs.txt"

using namespace std;

vector<string> rand_seed;
vector< vector<string> > attack_template;

mutex nxt_tmpl_mtx;
vector<pair<int, vector<vector<string> > > > next_template;


// Function to check if the first value's index is smaller than the third 
// value's index in the Hamiltonian path
template <typename T>
bool 
is_frontrunnable(const vector<T>& path, T firstValue, T thirdValue) 
{
    auto firstPos = find(path.begin(), path.end(), firstValue);
    auto thirdPos = find(path.begin(), path.end(), thirdValue);

    if (firstPos == path.end() || thirdPos == path.end()) {
        cerr << "Error: Values not found in the path." << endl;
        return false;
    }
    return firstPos < thirdPos;
}


void
add_tx(vector<string> &src_v, vector<string> &dest_v, char tx)
{
    for(string ordering : src_v){
        for(int i = 0; i < ordering.size(); i++){
            string new_ordering = ordering;
            new_ordering.insert(i, 1, tx);
            dest_v.push_back(new_ordering);
        }
        string new_ordering = ordering;
        new_ordering.insert(ordering.size(), 1, tx);
        dest_v.push_back(new_ordering);
    }
}


// Function to get the index of the target in the source vector
int get_index(vector<string>& src, string target) 
{
    auto it = find(src.begin(), src.end(), target);
    if (it == src.end()) return -1;
    return it - src.begin();
}


/*
 * function to start multi-threading
 * Parameter curr_ind indicate the index of the thread and it also indicate the
 * index of the ordering of adv_ord. Once this function is given a curr_ind, it
 * will only work on the adversary's ordering (adv_ord[curr_ind]), and permute
 * benign_ord.
 */
int
thread_permute_benign_ord(
    const size_t curr_ind, 
    const size_t node_cnt,
    const size_t cmd_cnt,
    const size_t rand_seed_cnt,
    const char tx_to_permute, 
    int prev_highest_succ_cnt
){
    vector< vector<string> > permuted_loc_ords;
    for(string loc_ord : attack_template[curr_ind]){
        vector<string> dest, src = {loc_ord};
        add_tx(src, dest, tx_to_permute);
        permuted_loc_ords.push_back(dest);
    }

    size_t highest_succ_cnt = prev_highest_succ_cnt;
    size_t lowest_fail_cnt = rand_seed_cnt - prev_highest_succ_cnt;
    size_t ordering_cnt = permuted_loc_ords[0].size();
    /* this array indicate each benign replica's loc_ord. if benign[0] == 3, it
     * means the 0th benign replica's loc_ord is equivalent to benign_ord[3].
     * if benign_ord_ind[benign_node_cnt] == 1, then return */
    size_t ord_ind[node_cnt+1]; 
    vector< vector<string> > local_highest_succ_tmpl;
    memset(ord_ind, 0, sizeof(ord_ind));

    
    while(ord_ind[node_cnt] != 1){
        /* constructs edge and graph for node_cnt times of tie-breaks */
        int weighted_edge[cmd_cnt][cmd_cnt];
        int edge[cmd_cnt][cmd_cnt];
        /* ord_in_int is an int array that translated each char ordering 
         * represented in int. adversary is indexed 0, first benign replica is 1
         * ord_represented_in_int[1][2] == 5 mean first replica's third cmd is
         * same with sixth cmd in adversary's ordering */
        int ord_in_int[node_cnt][cmd_cnt]; 
        memset(weighted_edge, 0, sizeof(weighted_edge));
        memset(edge, 0, sizeof(edge));

        /* applying replica's loc_ord to weighted edge */
        for(int i = 0; i < node_cnt; i++){
            for(int j = 0; j < cmd_cnt; j++){
                ord_in_int[i][j] = permuted_loc_ords[0][0].find(permuted_loc_ords[i][ord_ind[i]][j]);
                if(ord_in_int[i][j] == -1) {
                    fprintf(stderr, "expected index, got -1\n");
                    return -1;
                }
            }
            for(int j = 0; j < cmd_cnt -1; j++){
                for(int k = j+1; k < cmd_cnt; k++){
                    weighted_edge[ord_in_int[i][j]][ord_in_int[i][k]]++;
                }
            }
        }

        /* construct unweighted edge based on weighted_edge */
        for(int i = 0; i < cmd_cnt - 1; i++){
            for(int j = i+1; j < cmd_cnt; j++){
                if(weighted_edge[i][j] > weighted_edge[j][i])
                    edge[i][j] = 1;
                else edge[j][i] = 1;
            }
        }

        /* Repeat r times. Input every random seed. 
         * The previously contructed graph is translated based on the random 
         * seed. Then tie-break is preformed.  */
        int succ_cnt = 0, fail_cnt = 0;
        for(int i = 0; i < rand_seed_cnt; i++){
            vector<char> cmd_content;
            /* if adv_cmd_in_seed[0] == 3, it means adversary's 0th cmd is 
             * located at 3 in the seed */
            int adv_cmd_in_seed[cmd_cnt]; 
            Aequitas::TopologyGraph graph(cmd_cnt+1, 1, cmd_cnt);

            for(auto cmd : rand_seed[i]){ cmd_content.push_back(cmd); }

            for(int j = 0; j < cmd_cnt; j++){
                adv_cmd_in_seed[permuted_loc_ords[0][0].find(rand_seed[i][j])] = j;
            }
            for(int j = 0; j < cmd_cnt - 1; j++){
                for(int k = j+1; k < cmd_cnt; k++){
                    if(edge[j][k] == 1){
                        graph.addedge(adv_cmd_in_seed[j]+1, adv_cmd_in_seed[k]+1);
                    }
                    else {
                        graph.addedge(adv_cmd_in_seed[k]+1, adv_cmd_in_seed[j]+1);
                    }
                }
            }

            int TF_dist = 0;
            int TF_cycle_size = 0, cycle_cnt = 0;
            float avg_cycle_size = 0;
            int T_pos = 0, F_pos = 0;
            for(int j = 0; j < rand_seed[i].length(); j++){
                if(rand_seed[i][j] == 'T') T_pos = j+1;
                else if (rand_seed[i][j] == 'F') F_pos = j+1;
            }
            vector<char> final_ordered_cmds = graph.finalize(
                cmd_content, 
                TF_cycle_size,
                avg_cycle_size,
                cycle_cnt,
                TF_dist,
                T_pos, F_pos,
                NULL);

            if(TF_cycle_size != cmd_cnt){ break; }
            if (is_frontrunnable(final_ordered_cmds, TXF, TXT)){
                succ_cnt++;
            }
            else{
                fail_cnt++;
                if(fail_cnt > lowest_fail_cnt){
                    break;
                }
            }
        }

        /* if succ_cnt is higher or equal to the highest succ_cnt, record */
        if(succ_cnt != 0 && succ_cnt >= highest_succ_cnt){
            if(succ_cnt > highest_succ_cnt){
                highest_succ_cnt = succ_cnt;
                lowest_fail_cnt = rand_seed_cnt - highest_succ_cnt;
                local_highest_succ_tmpl.clear();
            }
            vector<string> highest_succ_tmpl;
            for(int i = 0; i < node_cnt; i++){
                highest_succ_tmpl.push_back(permuted_loc_ords[i][ord_ind[i]]);
            }
            local_highest_succ_tmpl.push_back(highest_succ_tmpl);
        }

        /* update index of benign replicas' ordering */
        for(int i = 0; i < node_cnt+1; i++){
            ord_ind[i]++;
            if(ord_ind[i] == ordering_cnt){ ord_ind[i] = 0; }
            else { break; }
        }
    }
    {
        lock_guard<mutex> lock(nxt_tmpl_mtx);
        next_template.push_back({highest_succ_cnt, local_highest_succ_tmpl});
    }
    return highest_succ_cnt;
}


void
thread_main(
    const size_t thread_cnt,
    const size_t thread_id,
    const size_t template_cnt,
    const size_t node_cnt,
    const size_t cmd_cnt,
    const size_t rand_seed_cnt,
    const char tx_to_permute, 
    int highest_succ
) {
    int processed = 0;
    for(int i = thread_id; i < template_cnt; i += thread_cnt){
        int local_highest_succ = thread_permute_benign_ord(
            i, 
            node_cnt, 
            cmd_cnt, 
            rand_seed_cnt, 
            tx_to_permute,
            highest_succ);
        if(local_highest_succ > highest_succ) highest_succ = local_highest_succ;
        if(thread_id == 0){
            processed++;
            if(processed % 100 == 0) 
                cout << "thread 0 processed " << processed << " tasks\n";
        }
    }
}


void
update_rand_seed(char tx_to_permute)
{
    vector<string> dest;
    add_tx(rand_seed, dest, tx_to_permute);
    rand_seed = dest;
}


void
initialize(size_t &cmd_cnt)
{
    attack_template.push_back(vector<string>());
    attack_template[0].push_back(string("FAT"));
    attack_template[0].push_back(string("ATF"));
    attack_template[0].push_back(string("TFA"));

    cmd_cnt = attack_template[0][0].size();

    rand_seed.push_back(string{attack_template[0][0][0]});
    for(int i = 1; i < attack_template[0][0].size(); i++){
        vector<string> dest;
        add_tx(rand_seed, dest, attack_template[0][0][i]);
        rand_seed = dest;
    }
}


int
record_result(
    const chrono::steady_clock::time_point begin,
    const size_t cmd_cnt,
    const size_t highest_succ_cnt,
    const size_t seed_size,
    const size_t atk_tmpl_size
) {
    char filename[64];
    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    sprintf(filename, RESULT_NAME, cmd_cnt);
    ofstream result(filename);
    if(!result.is_open()){
        fprintf(stderr, "failed to open result file %s\n", filename);
        return -1;
    }

    result << "cmd_cnt: " << cmd_cnt << "\n";
    result << "succ rate: " << highest_succ_cnt << " / " << seed_size << "\n";
    result << "started with " << atk_tmpl_size << " templates and got " 
        << attack_template.size() << " resulting templates\n";
    result << "it took " << chrono::duration_cast<chrono::minutes> (end-begin).count() 
        << " minutes to get here\n";
    cout << "elapsed time: " 
        << chrono::duration_cast<chrono::minutes> (end-begin).count() 
        << " minutes\n\n";

    for(vector<string> orderings : attack_template){
        for(string ordering : orderings){
            result << ordering << " ";
        }
        result << "\n";
    }
    result.close();
    return 0;
}


int 
main(int argc, char* argv[]) 
{
    float succ_rate = 0;
	size_t node_cnt = 3, cmd_cnt = 0, highest_succ_cnt = 0;
    size_t atk_tmpl_size = 0, seed_size = 0;
    vector<thread> threads;
    chrono::steady_clock::time_point begin = chrono::steady_clock::now(), end;
    vector<char> dummy_txs = {'X', 'Y', 'Z', 'V', 'W', 'S', 'U', 'Q', 'R'};

    size_t thread_cnt = thread::hardware_concurrency();
    cout << "thread cnt : " << thread_cnt << "\n";

    initialize(cmd_cnt);

    for(int i = 0; i < dummy_txs.size(); i++){
        update_rand_seed(dummy_txs[i]);
        cmd_cnt++;
        atk_tmpl_size = attack_template.size();
        seed_size = rand_seed.size();
        
        cout << "cmd cnt: " << cmd_cnt << "\n";
        cout << "total " << atk_tmpl_size << " templates\n";

        int highest_succ = int(seed_size * succ_rate);
        threads.clear();
        for(int j = 0; j < thread_cnt; j++){
            threads.push_back(
                thread(thread_main, 
                    thread_cnt,
                    j, 
                    atk_tmpl_size,
                    node_cnt, 
                    cmd_cnt, 
                    seed_size,
                    dummy_txs[i],
                    highest_succ
            ));
        }

        /* waiting for all threads */
        for(thread& t : threads){ t.join(); }

        sort(
            next_template.begin(), 
            next_template.end(), 
            [](const auto& lhs, const auto& rhs){ return lhs.first > rhs.first; }
        );

        /* record result */
        highest_succ_cnt = next_template[0].first;
        printf("highest succ rate is %ld / %ld\n", highest_succ_cnt, seed_size);

        attack_template.clear();
        for(int i = 0; i < next_template.size(); i++){
            if(next_template[i].first < highest_succ_cnt) break;
            for(vector<string> orderings : next_template[i].second){
                attack_template.push_back(orderings);
            }
        }
        succ_rate = (float) highest_succ_cnt / seed_size;
        if(record_result(begin, cmd_cnt, highest_succ_cnt, seed_size, atk_tmpl_size) != 0){
            fprintf(stderr, "failed to recored result\n");
            return 0;
        }

        next_template.clear();
    }

	return 0;
}