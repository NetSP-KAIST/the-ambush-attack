# Themis with delayed gossiping
 This code repository includes a modified version of the Themis implementation. 
 To run the Themis instances and reproduce the results from the paper, please follow the same instructions as provided in the [Themis](https://github.com/anonthemis/themis-src-anon/tree/main) and [HotStuff](https://github.com/hot-stuff/libhotstuff) repositories. The process is the same. 

 To launch the instances, you need to generate your own ``replicas.txt`` and ``clients.txt``. Currently, the IP addresses are replaced with invalid ones. Then, run ``./gen_all.sh`` to create ``nodes.ini``. Also, to change the binary path or user name, modify the settings in ``libhotstuff/scripts/deploy/group_vars``.

 ## Results for Section 7: Practical Countermeasure

- Modify the values of ``gossip_probability`` and ``gossip_queue_size`` in ``hotstuff_app.cpp``

- If you set ``gossip_probability`` to ``0``, there will be no difference from the original Themis implementation.


The following is an example dataset obtained with the following configuration: ``gossip_probability`` set to ``0.1``, ``gossip_queue_size`` set to ``1``, number of clients set to ``8``, number of replicas to ``21``, and block size set to ``100``:

    [8100, 7600, 8000, 7900, 8000, 8200, 8501, 8399, 8000, 7800, 2101]
    lat = 131.266 ms
    lat = 119.491 ms

The throughput is provided in the first line of the result. The remaining values represent the end-to-end latency of a transaction (or command), with the last value reflecting the data after removing outliers. You can modify the ``max_async`` in ``scripts/deploy/group_vars/clients.yml`` to adjust the number of concurrently outstanding requests. 
