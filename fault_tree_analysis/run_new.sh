#!/bin/bash
node_count=$(cat unique_nodes.txt unique_parent_list | wc -l) 
g++ graph_modified.cc -o graph_modified
./graph_modified $node_count > fp_factor.log


