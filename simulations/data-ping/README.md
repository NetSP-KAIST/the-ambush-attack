## Dataset to simulate network latencies

### wp-simplified.csv 

We used WonderProxy dataset ([download here](https://wonderproxy.com/blog/a-day-in-the-life-of-the-internet/)) to simulate network latency. 

We only kept the necessary information from the original dataset:
* We used "avg" field as the RTT between two servers("source" and "destination") and removed other unused parts from the original dataset. 
* We only kept the locations that are reachable from all other locations. In other words, the servers in our file can reach all other servers with single hop. 


### aws-individual_pings.csv
We measured RTT using AWS. The file contains individual RTT measurements using `ping`. The servers in this file is also reachable from all other servers in one hop. 


### aws-ping_summary.csv
This file contains the summary of aws-individual_pings.csv. 