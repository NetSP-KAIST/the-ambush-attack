#!/bin/bash

# generate ordering
python3 src-gen_orderings/gen_dataset_aws.py 9 data-ordering-aws
python3 src-gen_orderings/gen_dataset_aws.py 21 data-ordering-aws
python3 src-gen_orderings/gen_dataset_aws.py 101 data-ordering-aws

# unspooling
python3 run_unspool.py ./bin/unspool ./data-ordering-aws/themis00 ./data-unspool-aws
python3 run_unspool.py ./bin/unspool ./data-ordering-aws/themis01 ./data-unspool-aws
python3 run_unspool.py ./bin/unspool ./data-ordering-aws/themis02 ./data-unspool-aws

# summarize
python3 summarize_res.py --dir ./data-unspool-aws