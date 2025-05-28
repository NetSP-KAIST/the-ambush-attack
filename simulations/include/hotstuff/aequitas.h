#ifndef _AEQUITAS_H
#define _AEQUITAS_H


#include <vector>
#include <map>
#include <queue>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <algorithm>
// #include "hotstuff/type.h"
// #include "hotstuff/entity.h"


namespace  Aequitas {

class TopologyGraph {
    private:
    int max_number_cmds;
    int distinct_cmd, distinct_cmd_r;
    int TF_cycle_index = 0;
    int num_of_edges = 0;
    int num_solid = 0;
    int cnt = 0, top = 0, scc = 0;

    std::vector<bool> already_ordered;
    std::vector<int> inDegree;

    std::vector<bool> inst;
    std::vector<int> bel;
    std::vector<int> dfn;
    std::vector<int> low;
    std::vector<int> stck;

    // edge[i][j] = 1 means there is an edge from i to j
    std::vector< std::vector<int> > edge; 
    std::vector<std::vector<int>> scc_vertex;
    std::vector<int> hami_path_of_scc;
    // Arrays for graph after scc
    std::vector<std::vector<int>> edge_with_scc;

    public:
    TopologyGraph(int max_number_cmds, int distinct_cmd, int distinct_cmd_r) 
        :max_number_cmds(max_number_cmds),
          distinct_cmd(distinct_cmd),
          distinct_cmd_r(distinct_cmd_r),
          already_ordered(max_number_cmds, false),
          inDegree(max_number_cmds, 0),
          inst(max_number_cmds, false),
          bel(max_number_cmds, 0),
          dfn(max_number_cmds, 0),
          low(max_number_cmds, 0),
          stck(max_number_cmds, 0),
          edge(max_number_cmds, std::vector<int>(max_number_cmds, 0)),
          scc_vertex(max_number_cmds),
          edge_with_scc(max_number_cmds)
    { } 
    ~TopologyGraph() = default;


    void 
    addedge(int i, int j)
    {
        if(edge[i][j] || edge[j][i]) return;
        if(edge[i][j] == 0) {
            num_of_edges++; 
        }
        edge[i][j] = 1;
    }


    int 
    min(int i, int j)
    {
        if (i < j) return i;
        else return j;
    }


    void 
    tarjan(int u)
    {
        dfn[u] = low[u] = ++cnt;
        stck[++top] = u;
        inst[u] = true;

        for (int v = 1; v < max_number_cmds; ++v) {
            if(!edge[u][v]){
                continue;
            } 
            if (!dfn[v]){
                tarjan(v);
                low[u] = min(low[u], low[v]);
            } 
            else if (inst[v]){
                low[u] = min(low[u], dfn[v]);
            } 
        }

        if (dfn[u] == low[u]) {
            ++scc;
            int v;
            do{
                v = stck[top--];
                bel[v] = scc;
                scc_vertex[scc].push_back(v);
                inst[v] = false;
            } while (v != u);
        }
    }


    void 
    find_scc()
    {
        for (int i = distinct_cmd; i <= distinct_cmd_r; i++){
            if (!dfn[i]){
                tarjan(i);
            }
        }
    }


    void 
    topology_sort()
    {
        for (int i = distinct_cmd; i <= distinct_cmd_r; i++) {
            for (int j = distinct_cmd; j <= distinct_cmd_r; j++) {
                if(!edge[i][j]) continue;
                int ii = bel[i], jj = bel[j];
                if(ii != jj) {
                    edge_with_scc[ii].push_back(jj);
                    ++inDegree[jj];
                }
            }
        }
    }


    /*
     * curr is a vertex contained in the scc;
     * u is the index of the scc;
     * scc_size is the size of the scc_vertex[u];
     * scc_vertex[i] is the vertices included in the ith scc. 
     */ 
    bool 
    hami(int starting_v, int u, int scc_size) 
    {
        int nxt_hami[max_number_cmds];
        std::memset(nxt_hami, -1, sizeof(nxt_hami));
        int head = starting_v;

        for(int i = 0; i < scc_size; i++) {
            int curr_v = scc_vertex[u][i];
            if(curr_v == starting_v) continue;
            if(edge[curr_v][head]) { // curr_v -> head
                nxt_hami[curr_v] = head;
                head = curr_v;
            } 
            else if(edge[head][curr_v]){ // head -> curr_v
                /*
                 * pos is the next vertex of pre (or: pre -> pos). currently, 
                 * pre->curr_v, and don't know the relationship between pos and
                 * curr_v. so loop until curr_v -> pos. and make it into a 
                 * path that is pre -> curr_vr -> pos.
                 */
                int pre = head, pos = nxt_hami[head];
                while(pos != -1 && !edge[curr_v][pos]) {
                    pre = pos;
                    pos = nxt_hami[pre];
                }
                nxt_hami[pre] = curr_v;
                nxt_hami[curr_v] = pos;
            }
            else { throw std::runtime_error("Failed to recognize the tournament..."); }
        }
        /*
         * up to this point, a hamiltonian path is constructed. but the ending vertex's 
         * is pointing -1. and there is no guarantee that the ending vertex has a path
         * to the starting vertex. 
         * e.g., F->A->Y->X->-1; there may exist a path which is X->F or it may be F->X
         * and Themis wants this path to be a hamiltonian path such that (F->A->Y->X->F).
         * if F->X, than themis is willing to reconstruct the path.
         */
        int l = head, r = 0;
        for(int i = l; i > 0; i = nxt_hami[i]) {
            if(r) {
                for(int j = l, k = r; ; k = j, j = nxt_hami[j]) { 
                    if(edge[i][j]) {
                        nxt_hami[k] = nxt_hami[r]; // nxt_hami[k] = i
                        if(k != r) nxt_hami[r] = l;
                        l = j;
                        r = i;
                        break;
                    }
                    /*
                     * if there was no path from i (the nxt_hami[r] vertex), then break.
                     * there was a smaller cycle from l to r, or head to r; but there was
                     * no path from i to the smaller cycle.
                     * l->...->r->i, but there were no path from i to l->...->r
                     */
                    if(j == r){ 
                        break; 
                    }
                }
            } 
            else if(edge[i][l]) { r = i; }
        }
        nxt_hami[r] = l;

        for(int i = nxt_hami[l]; i != -1; i = nxt_hami[i]) {
            hami_path_of_scc.push_back(i);
            if(i == l) break;
        }
        return true;
    }

    
    //find hamilton road in the strong connected component G.scc_vertex[u]
    // pair<ii> is (cycle len, dist)
    void
    find_hamilton(
        int u,
        bool (*func)(
            int, 
            const std::vector<int>&, 
            const std::vector< std::vector<int> >&, 
            std::vector<int>&)
    ){
        if(scc_vertex[u].size() <= 2) return;
        bool success = 0;
        int scc_size = scc_vertex[u].size();
        for(int j = 0; j < scc_size; j++) {
            hami_path_of_scc.clear();
            int cur = scc_vertex[u][j]; 
            if(func == NULL){
                if(hami(cur, u, scc_size) 
                    && edge[hami_path_of_scc[scc_size - 1]][hami_path_of_scc[0]]
                ){
                    success = 1;
                    for(int i = 0; i < scc_size; i++)
                        scc_vertex[u][i] = hami_path_of_scc[i];
                    break;
                }
                else { continue; }
            }
            else{
                /* the algorithm is not bounded to hamiltonian cycle. thus 
                 * removing cycle checking condition.. */
                if(func(scc_size, scc_vertex[u], edge, hami_path_of_scc)){
                    success = 1;
                    for(int i = 0; i < scc_size; i++)
                        scc_vertex[u][i] = hami_path_of_scc[i];
                    break;
                }
                else { continue; }
            }
        }
        if(!success) {
            throw std::runtime_error("Hamilton Algorithm failed to find a road in SCC...");
        }
    }
     
    bool 
    is_tournament()
    {
        int k = num_solid; // since only the solid tx could be addedge-ed
        if(num_of_edges == k * (k - 1) / 2) return true;
        return false;
    }

    int 
    TF_coexisting_cycle_size(int T_pos, int F_pos)
    {
        int cycle_size = 0;
        for(int i = 0; i < max_number_cmds; i++){
                bool t_exist = 0, f_exist = 0;
                for(int j =0; j < scc_vertex[i].size(); j++){
                    if(scc_vertex[i][j] == T_pos){
                        t_exist = 1;
                    }
                    else if(scc_vertex[i][j] == F_pos){
                        f_exist = 1;
                    }
                }
                if(t_exist == 1 && f_exist == 1){
                    cycle_size = scc_vertex[i].size();
                    TF_cycle_index = i;
                }
            }
        return cycle_size;
    }

    int
    TF_distance(int T_pos, int F_pos)
    {
        int cycle_length = 0; int dist = 0;
        cycle_length = hami_path_of_scc.size();
        auto it_T = std::find(hami_path_of_scc.begin(), hami_path_of_scc.end(), T_pos);
        auto it_F = std::find(hami_path_of_scc.begin(), hami_path_of_scc.end(), F_pos);
        
        if(it_T != hami_path_of_scc.end() && it_F != hami_path_of_scc.end()) {
            dist = std::distance(it_F, it_T);
        } else {
            if(it_T == hami_path_of_scc.end()) {
                std::cout << "T_pos not found in hami_path_of_scc\n";
            }
            if(it_F == hami_path_of_scc.end()) {
                std::cout << "F_pos not found in hami_path_of_scc\n";
            }
        }
        
        if (dist < 0) return cycle_length + dist;
        else return dist;

    }

    /*
     * T_pos: the position of T in cmd_content. the index start from 1 instead 
     * of 0. If one of T_pos or F_pos is non-zero value and cycle_size is 
     * initilized with zero, the code will find T and F and cycle_size in the
     * local ordering.
     * 
     * F_pos: the position of F in cmd_content. the index start from 1 instead 
     * of 0. If one of T_pos or F_pos is non-zero value and cycle_size is 
     * initilized with zero, the code will find T and F and cycle_size in the
     * local ordering.
     * 
     * func: If func is not NULL, then the code will try to tie-break with the 
     * given func().
     */
    template <typename T>
    std::vector<T>
    finalize(
        std::vector<T> &cmd_content, 
        int &TF_cycle_size, 
        float &avg_cycle_size,
        int &cycle_cnt,
        int &TF_dist,
        int T_pos,
        int F_pos,
        bool (*func)(
            int, 
            const std::vector<int> &, 
            const std::vector< std::vector<int> > &, 
            std::vector<int> &)
    ) {
        find_scc();
        topology_sort();
        if(!(T_pos == 0 && F_pos == 0)){
            TF_cycle_size = TF_coexisting_cycle_size(T_pos, F_pos);
        }
    
        std::vector<T> final_ordered_cmds; final_ordered_cmds.clear();
        std::queue<int> que = std::queue<int>();
        int check_whether_all_cmds_are_ordered = 0;
        int tot_scc_size = 0;
        int max_circle_size = -1;
        int min_circle_size = max_number_cmds;

        for (int i = 1; i <= scc; i++) {
            // in_degree == 0 means the scc should be starting scc of all txs
            // there can be multiple in_degree == 0 sccs
            if (inDegree[i] == 0) que.push(i); 

            int cur_scc_size = (int)scc_vertex[i].size();

            tot_scc_size += cur_scc_size;

            if(cur_scc_size > max_circle_size) max_circle_size = cur_scc_size;
            if(cur_scc_size < min_circle_size) min_circle_size = cur_scc_size;
        }

        avg_cycle_size = (float)tot_scc_size / scc;
        cycle_cnt = scc;

        while(!que.empty()) {
            std::vector<int> to_be_added; to_be_added.clear();
            while(!que.empty()) {
                int u = que.front(); que.pop(); // que of sccs
                find_hamilton(u, func);
                if(u == TF_cycle_index) {
                    TF_dist = TF_distance(T_pos, F_pos);
                }
                for (int i = 0; i < scc_vertex[u].size(); i++) {
                    already_ordered[scc_vertex[u][i]] = 1;
                    final_ordered_cmds.push_back(cmd_content[scc_vertex[u][i] - 1]); 
                    check_whether_all_cmds_are_ordered++;
                }

                for (int i = 0; i < edge_with_scc[u].size(); i++) {
                    int v = edge_with_scc[u][i];
                    --inDegree[v];
                    if(inDegree[v] == 0)
                        to_be_added.push_back(v);
                }
                edge_with_scc[u].clear();
            }
            for(int i = 0; i < to_be_added.size(); i++)
                que.push(to_be_added[i]);
        }

        if (check_whether_all_cmds_are_ordered != (distinct_cmd_r - distinct_cmd + 1))
            throw std::runtime_error("Aequitas failed to topology sort the commands...");
        return final_ordered_cmds;
    }
};
}

#endif