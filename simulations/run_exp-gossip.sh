#!/bin/bash

# generate ordering
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 1 --gossip_p 0.01 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 5 --gossip_p 0.01 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 10 --gossip_p 0.01 --txs 300

python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 1 --gossip_p 0.03 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 5 --gossip_p 0.03 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 10 --gossip_p 0.03 --txs 300

python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 1 --gossip_p 0.05 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 5 --gossip_p 0.05 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 10 --gossip_p 0.05 --txs 300

python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 1 --gossip_p 0.1 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 5 --gossip_p 0.1 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 10 --gossip_p 0.1 --txs 300

python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 1 --gossip_p 0.2 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 5 --gossip_p 0.2 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 10 --gossip_p 0.2 --txs 300

python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 1 --gossip_p 0.3 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 5 --gossip_p 0.3 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 10 --gossip_p 0.3 --txs 300

python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 1 --gossip_p 0.5 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 5 --gossip_p 0.5 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 10 --gossip_p 0.5 --txs 300

python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 1 --gossip_p 1.0 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 5 --gossip_p 1.0 --txs 300
python3 src-gen_orderings/gen_gossiped_dataset.py 21 ./data-ordering-gossip --batch_size 10 --gossip_p 1.0 --txs 300

# unspooling
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis00 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis01 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis02 ./data-unspool-gossip

python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis03 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis04 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis05 ./data-unspool-gossip

python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis06 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis07 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis08 ./data-unspool-gossip

python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis09 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis10 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis11 ./data-unspool-gossip

python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis12 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis13 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis14 ./data-unspool-gossip

python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis15 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis16 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis17 ./data-unspool-gossip

python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis18 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis19 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis20 ./data-unspool-gossip

python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis21 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis22 ./data-unspool-gossip
python3 run_unspool.py bin/unspool ./data-ordering-gossip/themis23 ./data-unspool-gossip

# summarize
python3 summarize_res.py --dir ./data-unspool-gossip