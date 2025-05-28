#!/bin/bash

# generate ordering
python3 src-gen_orderings/gen_dataset_wp.py 9 data-ordering-wp
python3 src-gen_orderings/gen_dataset_wp.py 21 data-ordering-wp
python3 src-gen_orderings/gen_dataset_wp.py 101 data-ordering-wp

# unspooling
python3 run_unspool.py ./bin/unspool ./data-ordering-wp/themis00 ./data-unspool-wp
python3 run_unspool.py ./bin/unspool ./data-ordering-wp/themis01 ./data-unspool-wp
python3 run_unspool.py ./bin/unspool ./data-ordering-wp/themis02 ./data-unspool-wp

# summarize
python3 summarize_res.py --dir ./data-unspool-wp