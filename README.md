Artifact for "Collective Allocator Abstraction to Control Object Spatial Locality in C++"
===

* Artifact submission for #11 "Collective Allocator Abstraction to Control Object Spatial Locality in C++", which has been accepted to Programming 8.3.
  * authors: Takato Hideshima, Shigeyuki Sato, Tomoharu Ugawa
* URL to artifact: TODO
  * sha256 hash: `TODO`


## Getting Started Guide
### Components

The artifact is provided as a Docker image.
The image contains three main components:

  * The collective allocator library, which is the artifact.
  * Variety of B-tree and skip list implementations using the collective allocator library. They are listed in Section 5.2 in the paper.
  * Benchmarking related programs and scripts, including benchmark drivers (programs that uses the B-trees and the skip lists).

In addition, a git repository of [UMap](git@github.com:farmalloc/umap.git), a user-level page cache library, with our bug fix (commit cb294ef) is included.
These components are placed in the following directories in the default user's home directory `/workdir`:

  * `library`: the collective allocator library
  * `library/farmalloc_impl/umap`: UMap library
  * `include/far_memory_container`: B-tree and skip list implementations
    |container variant|implementation|
    |:-|:-|
    |`hint` B-tree|`include/far_memory_container/baseline/b_tree.hpp` <br> `include/far_memory_container/baseline/b_tree.ipp` <br> `include/far_memory_container/baseline/b_tree_node.ipp` <br> `include/far_memory_container/baseline/b_tree_iterator.ipp`|
    |`local` B-tree <br> `local+dfs` B-tree <br> `local+vEB` B-tree|`include/far_memory_container/blocked/b_tree.hpp` <br> `include/far_memory_container/blocked/b_tree.ipp` <br> `include/far_memory_container/blocked/b_tree_node.ipp` <br> `include/far_memory_container/blocked/b_tree_iterator.ipp`|
    |`dfs` B-tree <br> `vEB` B-tree|`include/far_memory_container/page_aware/b_tree.hpp` <br> `include/far_memory_container/page_aware/b_tree.ipp` <br> `include/far_memory_container/page_aware/b_tree_node.ipp` <br> `include/far_memory_container/page_aware/b_tree_iterator.ipp`|
    |`hint` skip list|`include/far_memory_container/baseline/skiplist.hpp`|
    |`local` skip list <br> `local+page` skip list|`include/far_memory_container/blocked/skiplist.hpp`|
    |`page` skip list|`include/far_memory_container/page_aware/skiplist.hpp`|
  * `include`: benchmarking related programs (data generator)
  * `src`: benchmarking related programs (benchmark drivers)
  * `scripts`: benchmarking related programs (scripts)

### Play around

Let's install the building environment, build the artifact, and play around the benchmark program.

#### Installation

The source code of the artifact and all necessary packages are installed in the Docker image.

  1. Download the ZIP file from [here](TODO: link) and unpack it. The Docker image `collective_farmalloc` and a security option file `docker_seccomp.json` will be extracted.
  2. Run the Docker image with the following commands.

```bash
sudo sysctl vm.unprivileged_userfaultfd=1
docker run -it \
  --security-opt seccomp=./docker_seccomp.json \
  collective_farmalloc  # name of docker image
```

Note: Because the far-memory library calls the `userfaultfd` system call,
we allow it by using `sysctl` and the security option (`--security-opt`). You can confirm validity of the security
option in `docker_seccomp.json` by comparing it with the [default setting](https://github.com/moby/moby/blob/2a38569337f97168792b8c0b5dd606032fe1dcac/profiles/seccomp/default.json).

<!-- リポジトリのREADMEを兼ねたいので、邪魔にならないようにこちらも書く -->
<details>
<summary>Try without Docker</summary>

#### Prerequisites

  * Linux (ubuntu 22.04)
  * CMake (version 3.13<!-- TODO: check --> or later)
  * Make or Ninja
  * C++ compiler (C++20 support is required; tested with gcc 12.3.0)

#### Download the artifact

Clone our git repository and its submodules recursively.
All the artifact files installed in the Docker image will be downloaded in `exp_clloective_farmalloc` directory.

```bash
$ git clone --recurse-submodules git@github.com:farmalloc/exp_collective_farmalloc.git
```

</details>


#### Build

Build the artifact and benchmarking programs with the following commands.

```bash
/workdir$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
# to use Makefile: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
/workdir$ cmake --build build/
```

### Testing the Artifact

Let's execute a benchmark program of a B-tree using our collective allocator library.
Here, we use the `local+dfs` variant of the B-tree (see Section 5.2) and
the "key-value store benchmark" in Section 5.1.1 with parameterers $L=200$, $\alpha=1.3$ and $U=0.05$.
This reproduces the result corresponding to the right most dark gray (purely-local & page-aware (dfs))<!-- TODO: errataに応じてlocal+dfsにする？ -->
plot in Figure 10c in the paper.

```bash
/workdir$ ./scripts/kvs_benchmark.sh btree local+dfs 200 1.3 0.05
```

The output will look like the follwings. If everything went well, the sum of the last two
numbers is around 5000 (4953 + 0 in this example).

```
###analyze_edges_of_local+dfs_btree###
#NumElements    NIteration      ZipfSkewness    UpdateRatio     PurelyLocalCapacity[B]  UMAP_BUFSIZE[pages]     batch_blocking  construction_duration[ns]       query_duration[ns] query_read_cnt  query_write_cnt
13421773        10000   1.3     0.05    2147483712      524288  1       71352077629     630600441       4953    0
```

This is the end of the getting started guide.

## Overview of Claims

論文のEvaluation (5章) 冒頭に書いた通り、claimは次の2つである。

1.  Our collective allocator abstraction is sufficiently expressive to implement various object placement strategies in a modular manner.
    * step-by-step instructions -> [Expressiveness](#expressiveness)
2.  Using the collective allocator appropriately for object placement aware of the far-memory model actually benefits to reducing remote swapping.
    * step-by-step instructions -> [Reduction of Remote Swapping](#reduction-of-remote-swapping)


## Step-by-Step Instructions
### Expressiveness

> Our collective allocator abstraction is sufficiently expressive to implement various object placement strategies in a modular manner.

このclaimをサポートする要素は次の3つである。

* collective allocatorの上の様々なB木・スキップリスト実装 (5.2章)
* その実装にかかるコード差分が小さかったこと (図7, 8, 13, 14, 15)
  * 行数での比較 (表3)
* その実装がオブジェクト間リンクの分析 (5.1.2章) において望ましい変化を見せたこと (図9)

以下、この順に、再現・確認の方法を述べる。

#### Container Implementations

コンテナ実装の各variantがどこで実装されているかは[Components](#components)の表の通りである。

#### Figure 7, 8

論文の図7の疑似コードは、B木実装においてcollective allocatorを使ってpurely-local aware placementを実装するための差分を示している。また図8の疑似コードは、図7をベースとして、page-aware placementを保存するための差分を示している。結果としてできあがる図8中の関数 `ins_rec` は、 `include/far_memory_container/blocked/b_tree.ipp` の190行目にある `BTreeMap::insert_step` に対応している。

また図7, 8中の主な追加行は、実際の実装と下のように対応している。

|図7中の行番号|`b_tree.ipp` 内の行番号|
|-:|-:|
|18|232|
|20, 21|254, 255|
|22|233|
|27|237, 326, 328|
|29|247|
|31, 32|254, 255, 327|
|34|245|
|35|247|
|40|258|
|41|259|

|図8中の行番号|`b_tree.ipp` 内の行番号|
|-:|-:|
|8|197, 206|
|15, 16, 17|198, 234|

#### Figure 13
#### Figure 14
#### Figure 15
#### Table 3
#### Figure 9

図9は `scripts/analyze_edges.sh` を用いて再現できる。例えば図9aの `hint` の結果は次のコマンドで再現できる。

```bash
/workdir$ ./scripts/analyze_edges.sh btree hint
```

上記のコマンドは下のような出力をする。

```
###analyze_edges_of_local+dfs_btree###
#NumElements    PurelyLocalCapacity[B]  batch_blocking  construction_duration[ns]       purely_local_edges      same_page_edges diff_pages_edges
13421773        2147483712      1       85143447311     5592382 0       4473384
```

出力中の右から1, 2, 3番目の値が、それぞれ *cross-page link*, *in-page link*, *purely-local link* の個数を示している。図9aに書かれた数字はこれらの比率である。

他の結果は `scripts/analyze_edges.sh` に与えるパラメータを変えることで再現される。当該スクリプトのパラメータの意味は下記の通りである。コマンド例の `btree` を `skiplist` に変えると図9bの結果が得られるようになり、 `hint` を `local` など他のcontainer variantの名前に変えると対応する結果が得られる。

```
Usage: analyze_edges.sh STRUCTURE OBJ_PLMT

Parameters
    STRUCTURE:  one of {btree, skiplist}
    OBJ_PLMT:   when STRUCTURE is btree, one of {hint, local, local+dfs, dfs,
                                                 local+veb, veb}
                when STRUCTURE is skiplist, one of {hint, local, local+page,
                                                    page}
```

このスクリプトは、 `src` ディレクトリの下のソースファイルをコンパイルしたものを実行している。


### Reduction of Remote Swapping

```bash
/workdir$ ./scripts/kvs_benchmark.sh btree local+dfs 200 1.3 0.05
```

```
###analyze_edges_of_local+dfs_btree###
#NumElements    PurelyLocalCapacity[B]  batch_blocking  construction_duration[ns]       purely_local_edges      same_page_edges diff_pages_edges
13421773        2147483712      1       85143447311     5592382 0       4473384
```

```
Usage: kvs_benchmark.sh STRUCTURE OBJ_PLMT LOCAL_MEM_CAP ZIPF_SKEWNESS UPDATE_RATIO

Parameters
    STRUCTURE:      one of {btree, skiplist}
    OBJ_PLMT:       when STRUCTURE is btree, one of {hint, local, local+dfs, dfs,
                                                     local+veb, veb}
                    when STRUCTURE is skiplist, one of {hint, local, local+page,
                                                        page}
    LOCAL_MEM_CAP:  PERSENTAGE (unsignd integer) of local memory usage to the
                    total data size
    ZIPF_SKEWNESS:  one of {0.8, 1.3}
    UPDATE_RATIO:   one of {0.05, 0.5}
```
