#!/bin/bash
cd $(dirname $0)


function for_all_benchmark_params() {
  for L in `seq 200 -5 1`
  do
    for alpha in 0.8 1.3
    do
      for U in 0.05 0.5
      do
        ./kvs_benchmark.sh $1 $2 $L $alpha $U
      done
    done
  done
}

for_all_benchmark_params btree hint
for_all_benchmark_params btree local
for_all_benchmark_params btree local+dfs
for_all_benchmark_params btree dfs
for_all_benchmark_params btree local+veb
for_all_benchmark_params btree veb

for_all_benchmark_params skiplist hint
for_all_benchmark_params skiplist local
for_all_benchmark_params skiplist local+page
for_all_benchmark_params skiplist page
