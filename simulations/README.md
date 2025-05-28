# Ambush attack simulation
This code repository includes the following:

### simulation code (`src-gen_orderings/`):  
The codes uses the network latency dataset in `data-ping` to simulate benign user transaction submission and adversary's ambush attack attemps. The adversary's transaction template can be found in `data-templates`. The generated datasets (local sequences) are stored in `data-ordering-*/`.
* `gen_dataset_aws.py` uses our aws ping measurements to simulate the transaction submission of benign users and the adversary. (Section 5.3.2)
* `gen_dataset_wp.py` uses WonderProxy ping measurements to simulate the transaction submission of benign users and the adversary. (Section 5.3.2)
* `gen_gossiped_dataset.py` is used to simulate our coutermeasure--delayed gossip. It uses WonderProxy ping measurements to simulate the network latency. (Section 6, 7)

### unspooling algorithm of Themis (`include/hotstuff/aequitas.h`):  
The code is imported from the Themis [github](https://github.com/anonthemis/themis-src-anon/tree/main/Aequitas-hotstuff/libhotstuff/include/hotstuff) directory. We modified the code sightly to improve the performance and clarity; the functionality and algorithm is *NOT* modified. 

### unspooling codes (`src-unspooling_algo`):  
The codes uses the Themis' unspooling algorithm to find transaction templates and unspool the input local sequences. The codes are compiled into the `bin` directory. 


## How to reproduce
We recommend to use Ubuntu 22.04 and python3.  
Use `make` to create directories and compile binaries.  
The required python libraries are listed in the `requirements.txt`.  

### Reproduce Figure 6 and Figure 7 (Section 5.3.1)
You can use `./bin/search_template` to find the templates optimized without benign transactions. 
The resulting templates are store under `data-found_templates/`. 

    $ ./bin/search_template 
    thread cnt : 96
    cmd cnt: 4
    total 1 templates
    highest succ rate is 9 / 24
    elapsed time: 0 minutes

    cmd cnt: 5
    total 20 templates
    highest succ rate is 74 / 120
    elapsed time: 0 minutes

    cmd cnt: 6
    total 39 templates
    highest succ rate is 616 / 720
    elapsed time: 0 minutes
        ... 

## Reproduce Figure 9, Fuigure 10, and Figure 12 (Section 5, 7)
You can use `run_exp-aws.sh`, `run_exp-wp.sh`, and `run_exp-gossip.sh` to simulate the transaction submission thus generating local sequences.  

    $ ./run_exp-wp.sh
        ...
    run: ./data-unspool-wp/themis00
    ./data-unspool-wp/themis00/0000 1333007 1400000 0.9521
    ./data-unspool-wp/themis00/0100 1193224 1400000 0.8523
    ./data-unspool-wp/themis00/0500 1292083 1400000 0.9229
    ./data-unspool-wp/themis00/1000 1367187 1400000 0.9766
    ./data-unspool-wp/themis00/1500 1370402 1400000 0.9789
    run: ./data-unspool-wp/themis01
    ./data-unspool-wp/themis01/0000 3939987 5000000 0.7880
    ./data-unspool-wp/themis01/0100 4354325 5000000 0.8709
    ./data-unspool-wp/themis01/0500 4881717 5000000 0.9763
    ./data-unspool-wp/themis01/1000 4961442 5000000 0.9923
    ./data-unspool-wp/themis01/1500 4957261 5000000 0.9915
    run: ./data-unspool-wp/themis02
    ./data-unspool-wp/themis02/0000 4034662 5000000 0.8069
    ./data-unspool-wp/themis02/0100 4489544 5000000 0.8979
    ./data-unspool-wp/themis02/0500 4894467 5000000 0.9789
    ./data-unspool-wp/themis02/1000 4977632 5000000 0.9955
    ./data-unspool-wp/themis02/1500 4985572 5000000 0.9971

