# GNU GENERAL PUBLIC LICENSE
# Version 3, 29 June 2007
# 
# Copyright (C) 2024 Ambush-attack-authors
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import csv
import argparse
import random
import os
import re
import numpy as np
import pandas as pd
import tqdm
from collections import deque
from multiprocessing import Pool

CONCURR = True
BATCH_SIZE = 0 # 0: no partial gossip; 1: partial gossip; more than 1: batched gossip
GOSSIP_P = 0.0
NUM_WORKER = 50

##### tx generation parameters#####
NUM_CLIENTS = 15
NUM_TXS = [0, 100, 500, 1000, 1500]
EPOCH = 1000
VICTIM_TX = 'T'
VICTIM_TX_GEN_TIME = 500  # The time when the victim sends the target transaction.
AMBUSH_INTERVAL = 10  # 1ms interval for ambush transactions
REORDERING_START = 700 # REORDERING_START is the time when the adversary puts his/her first template transaction in his/her own local ordering (for reordering)
REORDERING_INTERVAL = 10


##### global data structure shared between threads #####
OTT_DF = [] # one-way trip time data frame
UNIQUE_SRCS = []


##### file names #####
PING_DATASET_FILENAME = './data-ping/aws-individual_pings.csv'
TEMPLATE_DIR = './data-templates'


##### functions #####
def read_latency_file(rtt_filename):
    global OTT_DF, UNIQUE_SRCS
    rtt_df = []
    rtt_df = pd.read_csv(rtt_filename)
    rtt_df = rtt_df[['source', 'destination', 'time']]
    # ott stands for one-way trip time, 
    rtt_df['time'] = rtt_df['time'] / 2
    OTT_DF = rtt_df.rename(columns={'source': 'src', 
                                    'destination': 'dest', 
                                    'time': 'ott'})
    UNIQUE_SRCS = OTT_DF['src'].unique()


def write_results_to_csv(nodes_orderings, output_file, num_nodes, adv_node):
    with open(output_file, mode='w', newline='') as file:
        file.write(str(num_nodes) + "\n")
        local_ordering = ', '.join(f"{tx_id}" for tx_id, _ in nodes_orderings[adv_node]) + ','
        file.write(local_ordering + '\n')
        
        for (node, local_ordering) in nodes_orderings.items():
            if node == adv_node: continue
            local_ordering = ', '.join(f"{tx_id}" for tx_id, _ in local_ordering) + ','
            file.write(local_ordering + '\n')


def sample_locations(num_nodes, num_clients):
    all_nodes = random.choices(list(UNIQUE_SRCS), k=num_nodes)
    all_nodes = [f'{node}{idx:03}' for idx, node in enumerate(all_nodes)]
    adversary_node = random.choice(all_nodes)
    all_clients = random.sample(list(UNIQUE_SRCS), num_clients)
    victim_client = random.choice(all_clients)
    return all_nodes, adversary_node, all_clients, victim_client


# This function assumes that the victim's and the adversary's locations are all
# included in all_clients and all_nodes, respectively.
def sample_ott(all_clients, all_nodes):
    all_nodes = [node[:-3] for node in all_nodes]
    all_locations = all_clients + all_nodes
    filtered_df = OTT_DF[(OTT_DF['src'].isin(all_locations)) & 
                         (OTT_DF['dest'].isin(all_locations))]
    return filtered_df


def gen_client_txs(client_id, num_txs, start_time=0):
    txs = []
    if num_txs != 0:
        tx_gaps = np.random.exponential(1000.0/float(num_txs), num_txs)
        tx_gen_time = np.cumsum(tx_gaps)
        txs = [(f"{client_id:02}-{idx+1:03}", float(f"{start_time + gen_t:.2f}")) for idx, gen_t in enumerate(tx_gen_time)]
    return txs


def read_adv_reorder_csv_file(file_path):
    with open(file_path, mode='r') as file:
        csv_reader = csv.reader(file)
        row = next(csv_reader)
        return [tx.strip() for tx in row if tx.strip()]


def read_adv_txs_csv_file(file_path):
    adversary_transactions = []

    with open(file_path, mode='r') as file:
        csv_reader = csv.reader(file)
        for idx, row in enumerate(csv_reader):
            transactions = [tx.strip() for tx in row if tx.strip()]
            adversary_transactions.append(transactions)
    return adversary_transactions


def classify_transactions(transactions):
    t_index = transactions.index('T')
    ambush = transactions[:t_index]
    t = 'T'
    trapping = transactions[t_index + 1:]
    return ambush, t, trapping


def choose_rand_ott(src, dest, filtered_ott):
    if src[-3:].isdigit(): src = src[:-3]
    if dest[-3:].isdigit(): dest = dest[:-3]
    src_df = filtered_ott.get_group(src)
    ott_list = src_df[src_df['dest'] == dest]
    ind = random.randrange(0, ott_list.shape[0])
    return ott_list['ott'].values[ind]


def cal_tx_arrival_time(all_client_txs, all_nodes, filtered_ott):
    tx_arrival_time = {node:[] for node in all_nodes}
    for client_loc, tx_list in all_client_txs.items():
        for tx, gen_time in tx_list:
            for node_loc in all_nodes:
                ott = choose_rand_ott(client_loc, node_loc, filtered_ott)
                tx_arrival_time[node_loc].append((tx, gen_time + ott))
    return tx_arrival_time


def add_victim_tx(all_clients_txs, victim_client_loc):
    all_clients_txs[victim_client_loc].append((VICTIM_TX, VICTIM_TX_GEN_TIME))
    return all_clients_txs


def add_adv_txs(adv_txs, filtered_ott, nodes_orderings, all_nodes, adv_node):
    victim_tx_arr_time = 0
    for (tx, arr_time) in nodes_orderings[adv_node]:
        if tx == VICTIM_TX: 
            victim_tx_arr_time = arr_time
            break
    else: exit('adv got no victim TX')

    for node_loc, txs in adv_txs.items(): 
        ambush, _, trapping = classify_transactions(txs)
        
        for i, tx in enumerate(ambush):
            rand_ott = choose_rand_ott(adv_node, node_loc, filtered_ott)
            received_time = i * AMBUSH_INTERVAL + rand_ott
            nodes_orderings[node_loc].append((tx, received_time))

        rand_ott = choose_rand_ott(adv_node, node_loc, filtered_ott)
        trapping_interval = (EPOCH - victim_tx_arr_time - rand_ott) / (len(trapping))
        # using the following code, blobs adv's txs can be located at the end 
        # of local orderings
        # trapping_interval = (EPOCH - victim_tx_arr_time) / len(trapping)
        for i, tx in enumerate(trapping):
            rand_ott = choose_rand_ott(adv_node, node_loc, filtered_ott)
            received_time = victim_tx_arr_time + i*trapping_interval + rand_ott
            nodes_orderings[node_loc].append((tx, received_time))


def reorder_adv_txs(adv_reorder_txs, nodes_orderings, adv_node):
    nodes_orderings[adv_node] = [(tx, time) for tx, time in nodes_orderings[adv_node] if tx != VICTIM_TX]
    
    for i, tx in enumerate(adv_reorder_txs):
        received_time = REORDERING_START + i * REORDERING_INTERVAL
        nodes_orderings[adv_node].append((tx, received_time))


def apply_partial_gossip(nodes_orderings, adv_node, filtered_ott):
    honest_nodes = [node for node in nodes_orderings.keys() if node != adv_node]
    # merge all node orderings together into a list
    all_txs = [(node, tx, recv_time) for node, txs in nodes_orderings.items() if node != adv_node for tx, recv_time in txs]
    all_txs.sort(key=lambda x: x[2])
    q = deque(all_txs)

    gossiped_orderings = {node:[] for node in honest_nodes}
    gossip_queue = {node: {peer:[] for peer in honest_nodes if peer != node} for node in honest_nodes}
    while len(q) != 0:
        tx_dest_node, tx_content, tx_recv_time = q.popleft()
        if not any(tx_content == received_tx for received_tx, _ in gossiped_orderings[tx_dest_node]):
            gossiped_orderings[tx_dest_node].append((tx_content, tx_recv_time))
            # do partial (batched) gossip
            for peer in honest_nodes:
                if peer == tx_dest_node: continue
                p_to_peer = random.random() # [0.0, 1.0)
                if p_to_peer > GOSSIP_P: continue
                gossip_queue[tx_dest_node][peer].append(tx_content)
                # queue is full, gossip them
                if len(gossip_queue[tx_dest_node][peer]) >= BATCH_SIZE:
                    latency = choose_rand_ott(tx_dest_node, peer, filtered_ott)
                    for gossip_q_tx in gossip_queue[tx_dest_node][peer]:
                        gossiped_tx = (peer, gossip_q_tx, tx_recv_time+latency)
                        # insert to q while keeping q sorted
                        for idx, tx in enumerate(q):
                            if gossiped_tx[2] < tx[2]:
                                q.insert(idx, gossiped_tx)
                                break
                        else: q.append(gossiped_tx)
                        # to assure the gossiped tx stays in order
                        latency += 0.001
                    gossip_queue[tx_dest_node][peer] = []
                    # assert(len(gossip_queue[tx_dest_node][peer]) == 0)

    gossiped_orderings[adv_node] = nodes_orderings[adv_node]
    gossiped_orderings[adv_node].sort(key=lambda x: x[1])
    return gossiped_orderings


def run_wrapper(args):
    total_txs, num_clients, num_nodes, adv_txs_template, adv_reorder_txs, out_filename, i = args
    run(total_txs, num_clients, num_nodes, adv_txs_template, adv_reorder_txs, out_filename, i)


def run(total_txs, num_clients, num_nodes, adv_txs_template, adv_reorder_txs, out_filename, i):
    seed = total_txs + num_nodes*10000 + i*1000000
    random.seed(seed)
    np.random.seed(seed)
    all_nodes, adv_node, all_clients, victim_client = sample_locations(num_nodes, num_clients)
    filtered_ott = sample_ott(all_clients, all_nodes).groupby('src')

    adv_txs = {}
    honest_nodes = [node for node in all_nodes if node != adv_node]
    for idx, txs in enumerate(adv_txs_template):
        adv_txs[honest_nodes[idx]] = txs

    # generate client transactions
    clients_txs_gen_time = {}
    txs_per_client = int(total_txs / num_clients) +1
    remainder = total_txs % num_clients
    for client_id, region in enumerate(all_clients):
        if remainder == client_id: txs_per_client -= 1
        clients_txs_gen_time[region] = gen_client_txs(client_id+1, txs_per_client)
    clients_txs_gen_time = add_victim_tx(clients_txs_gen_time, victim_client)
    
    # add adversarial behaviors
    nodes_orderings = cal_tx_arrival_time(clients_txs_gen_time, all_nodes, filtered_ott)
    add_adv_txs(adv_txs, filtered_ott, nodes_orderings, all_nodes, adv_node)
    reorder_adv_txs(adv_reorder_txs, nodes_orderings, adv_node)

    if BATCH_SIZE == 0:
        # sort local orderings by time and write to file
        for local_ordering in nodes_orderings.items():
            local_ordering[1].sort(key=lambda x: x[1])
    else: 
        nodes_orderings = apply_partial_gossip(nodes_orderings, adv_node, filtered_ott)

    write_results_to_csv(nodes_orderings, out_filename, num_nodes, adv_node)



def chk_input_and_setup_dir():
    global GOSSIP_P
    global BATCH_SIZE
    parser = argparse.ArgumentParser(description='Generate dataset with given \
                                     parameters.')
    parser.add_argument('num_nodes', type=str, help='Number of nodes, including the \
                        adversary. Should be 9, 21, or 101.')
    parser.add_argument('output_dir', type=str, help='The output directory name. \
                        The script will create a sub-directory and store results\
                        under the newly created sub-directory.', default='./')

    args = parser.parse_args()
    num_nodes = args.num_nodes
    output_dir = args.output_dir

    if num_nodes not in ['9', '21', '101']:
        exit('num_node should be either \'9\', \'21\', or \'101\'')

    sub_dirs = [d for d in os.listdir(output_dir) 
                   if os.path.isdir(os.path.join(output_dir, d))]
    sub_dirs = [d for d in sub_dirs if 'themis' in d]
    sub_dirs.sort()
    if len(sub_dirs) == 0: new_dir = 'themis00'
    else:
        new_dir = 'themis' + str(int(re.search(r'\d+', sub_dirs[-1]).group())+1).zfill(2)
    new_dir = os.path.join(output_dir, new_dir)
    os.makedirs(new_dir)
    for num_txs in NUM_TXS:
        os.makedirs(os.path.join(new_dir, f'{num_txs:04}'))

    template_filename = os.path.join(TEMPLATE_DIR, f'themis-{num_nodes}.csv')
    reordered_template_filename = os.path.join(TEMPLATE_DIR, f'themis-{num_nodes}-reorder.csv')

    print('num_node: ', num_nodes)
    print('created datase: ', new_dir)

    return int(num_nodes), template_filename, reordered_template_filename, new_dir

def write_readme(home_dir, num_nodes, ping_filename, adv_txs_template, adv_reorder_txs):
    with open(os.path.join(home_dir, 'README.md'), mode='w', newline='\r\n') as file:
        file.write(f'num_nodes: {num_nodes}\n')
        file.write(f'epoch: {EPOCH}\n')
        file.write(f'victim tx gen time: {VICTIM_TX_GEN_TIME}\n')
        file.write(f'ambush interval: {AMBUSH_INTERVAL}\n')
        file.write(f'reordering start: {REORDERING_START}\n')
        file.write(f'reordering interval: {REORDERING_INTERVAL}\n')
        file.write(f'input dataset: {ping_filename}\n')
        file.write(f'batch size: {BATCH_SIZE}\n')
        file.write(f'gossip p: {GOSSIP_P}\n')
        file.write(f'adv_reorder_template: {adv_reorder_txs}\n')
        for row in adv_txs_template:
            file.write(f'adv_tx_template: {row}\n')

def concur_main():
    res = chk_input_and_setup_dir()
    num_nodes, template_filename, reordered_template_filename, home_dir = res

    if not os.path.exists(PING_DATASET_FILENAME):
        exit('ping file does not exist')
    read_latency_file(PING_DATASET_FILENAME)
    adv_txs_template = read_adv_txs_csv_file(template_filename)
    adv_reorder_txs = read_adv_reorder_csv_file(reordered_template_filename)

    write_readme(home_dir, num_nodes, PING_DATASET_FILENAME, adv_txs_template, adv_reorder_txs)

    inputs = []
    for num_tx in NUM_TXS:
        for i in range(50): 
            out_filename = os.path.join(home_dir, str(num_tx).zfill(4))
            out_filename = os.path.join(out_filename, str(i).zfill(2)+'.csv')
            inputs.append((num_tx, NUM_CLIENTS, num_nodes, adv_txs_template, adv_reorder_txs, out_filename, i))

    print(f'num workder: {NUM_WORKER}')
    with Pool(NUM_WORKER) as pool:
        with tqdm.tqdm(total=len(inputs)) as pbar:
            for res in pool.imap(run_wrapper, inputs):
                pbar.update()

def single_main():
    res = chk_input_and_setup_dir()
    num_nodes, template_filename, reordered_template_filename, home_dir = res

    if not os.path.exists(PING_DATASET_FILENAME):
        exit('ping file does not exist')
    read_latency_file(PING_DATASET_FILENAME)
    adv_txs_template = read_adv_txs_csv_file(template_filename)
    adv_reorder_txs = read_adv_reorder_csv_file(reordered_template_filename)

    write_readme(home_dir, num_nodes, PING_DATASET_FILENAME, adv_txs_template, adv_reorder_txs)

    num_tx = NUM_TXS[0]
    out_filename = os.path.join(home_dir, str(num_tx).zfill(4))
    out_filename = os.path.join(out_filename, str(0).zfill(2)+'.csv')
    run(num_tx, NUM_CLIENTS, num_nodes, adv_txs_template, adv_reorder_txs, out_filename, 0)


if __name__ == "__main__":
    if CONCURR:
        concur_main()
    else:
        single_main()
