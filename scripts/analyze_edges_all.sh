#!/bin/bash
cd $(dirname $0)

./analyze_edges.sh btree hint
./analyze_edges.sh btree local
./analyze_edges.sh btree local+dfs
./analyze_edges.sh btree dfs
./analyze_edges.sh btree local+veb
./analyze_edges.sh btree veb

./analyze_edges.sh skiplist hint
./analyze_edges.sh skiplist local
./analyze_edges.sh skiplist local+page
./analyze_edges.sh skiplist page
