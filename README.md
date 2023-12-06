Artifact for "Collective Allocator Abstraction to Control Object Spatial Locality in C++"
===

* authors: Takato Hideshima, Shigeyuki Sato, Tomoharu Ugawa
* submitted-to: ‹Programming› 2024

## Components

* `library`: far-memoryシステム向けcollective allocatorの実装
  * `library/farmalloc_impl/umap`: source code of [UMap](git@github.com:farmalloc/umap.git), an external library of user-level page cache
* `include/far_memory_container`: collective allocatorを使ったB木・スキップリストの実装の様々なバリエーション。論文の5.2章で示したものに対応する(下の表)
  |名前|実装しているファイル|
  |:-|:-|
  |`hint B-tree`|`include/far_memory_container/baseline/b_tree.hpp` <br> `include/far_memory_container/baseline/b_tree.ipp` ...|
  |TODO||
* `include`: ベンチマークで用いるデータ生成のプログラム
* `src`: 主にベンチマークプログラム集。詳細は[step-by-step instructions](#step-by-step-instructions)
  * corss-page linksの評価(図9)のための計測プログラム
  * スワップの回数を計測する(図10--12)プログラム
* `scripts`: パラメータを設定しベンチマークプログラムを実行するスクリプト集。詳細は[step-by-step instructions](#step-by-step-instructions)を参照。

なお、使用したバージョンのUMapにはバグがあったため修正を施した。詳しくは `git -C library/farmalloc_impl/umap diff cb294ef` を参照されたい。

## Getting Started Guide

ここでは、artifactのインストール、バイナリのビルド、実験の小さな一部の実行までの手順を記述する。

### Installation

\[dockerイメージ\]\(TODO: link\)を用いて次のコマンドを実行する。

```bash
sudo sysctl vm.unprivileged_userfaultfd=1
docker run -it \
  --security-opt seccomp=./docker_seccomp.json \
  collective_farmalloc  # name of docker image
```

`userfaultfd` システムコールを呼べるようにするために、カーネルとコンテナランタイムそれぞれでパラメータの設定が必要である。
コンテナランタイムの設定の[デフォルト値](https://github.com/moby/moby/blob/2a38569337f97168792b8c0b5dd606032fe1dcac/profiles/seccomp/default.json)からの差分は、 `syscalls > names` に `userfaultfd` を加えたことだけである。

<!-- リポジトリのREADMEを兼ねたいので、邪魔にならないようにこちらも書く -->
<details>
<summary>Gitを用いる場合</summary>

recursive cloneをする。

```bash
$ git clone --recurse-submodules git@github.com:farmalloc/exp_collective_farmalloc.git workdir
```

また、ビルドに向けて次の表にあるdependenciesを用意する。ubuntu 22.04では全てaptで入ることを確認済みである。

|Dependency|要件|
|:-|:-|
|CMake|バージョン3.13?(要確認)以上。|
|Make or Ninja||
|C++ Compiler|C++20をコンパイルできること。 `g++` v12.3.0での動作を確認している。|

</details>


### ビルド

下のコマンドを実行する。

```bash
/workdir$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
# Makefileを用いる場合: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
/workdir$ cmake --build build/
```

### Testing the Artifact

テストとして下のコマンドを実行し、論文の図10に示されるプロットの1点を再現する実験を実行する。

```bash
/workdir$ ./scripts/kvs_benchmark.sh btree local+dfs 200 1.3 0.05
```

これは `local+dfs B-tree` (5.2章) についてkey-value store benchmark (5.1.1章)を、$\alpha=1.3, U=0.05$というベンチマーク・パラメータで実行する。
出力例は下に示すとおりである。

```
###analyze_edges_of_local+dfs_btree###
#NumElements    NIteration      ZipfSkewness    UpdateRatio     PurelyLocalCapacity[B]  UMAP_BUFSIZE[pages]     batch_blocking  construction_duration[ns]       query_duration[ns] query_read_cnt  query_write_cnt
13421773        10000   1.3     0.05    2147483712      524288  1       71352077629     630600441       4953    0
```

右端の2つのcolumnに出力される値の合計がこの例と同程度であれば、適切に実行できている。

> この合計値は、当該ベンチマークにおいてスワップされたデータの量(単位はページ)であり、図10cの最も右下のプロットに対応する。


## Overview of Claims

論文のEvaluation (5章) 冒頭に書いた通り、claimは次の2つである。

1.  Our collective allocator abstraction is sufficiently expressive to implement various object placement strategies in a modular manner.
    * step-by-step instructions -> [Expressiveness](#expressiveness)
2.  Using the collective allocator appropriately for object placement aware of the far-memory model actually benefits to reducing remote swapping.
    * step-by-step instructions -> [Reduction of Remote Swapping](#reduction-of-remote-swapping)


## Step-by-Step Instructions
### Expressiveness
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
