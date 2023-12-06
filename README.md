Artifact for "Collective Allocator Abstraction to Control Object Spatial Locality in C++"
===

* authors: Takato Hideshima, Shigeyuki Sato, Tomoharu Ugawa
* submitted-to: ‹Programming› 2024

## Components

* `artifact`
  * `farmalloc_abst`: コンテナがcollective allocatorの機能を呼び出す際に用いる `collective_allocator_traits` の定義がある。これはC++標準のアロケータ抽象化における `std::allocator` を、collective allocator抽象化において具現化したものである。
  * `farmalloc_impl`: collective allocatorと、実験で比較のために用いたhint-onlyなアロケータの実装がある。
    * `umap`: remote swapping at the user levelの実装に用いた外部ライブラリUMapがsubmoduleとしてある。
  * `far_memory_container`: 論文の5.2章で述べたcontainer implementationsがある。
  * `util`: `farmalloc_impl` と `far_memory_container` で使われる小さなユーティリティがある。
  * `tests`
* `include`: 複数の実験で共有されるパラメータや処理の記述
* `src`: 主にベンチマークプログラム集
  * `read.cpp`: ベンチマークプログラムでない例外。データを値渡しで受け取って何もしない関数を別のソースファイルに分けることで、読み出しがコンパイル最適化によって消されないようにする
  * `analyze_edges_of_*.cpp`: 論文の図9のための計測プログラム。オブジェクト配置戦略の違いとB木/スキップリストの区別によって別のファイルに分かれている。
  * the others: key-value store benchmarkのプログラム。オブジェクト配置戦略の違いとB木/スキップリストの区別によって別のファイルに分かれている。
* `scripts`: パラメータを設定しベンチマークプログラムを実行するスクリプト集。詳細は[step-by-step instructions](#step-by-step-instructions)を参照。

なお、使用したバージョンのUMapにはバグがあったため修正を施した。詳しくは `git -C artifact/farmalloc_impl/umap diff cb294ef` を参照されたい。

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
## Step-by-Step Instructions
