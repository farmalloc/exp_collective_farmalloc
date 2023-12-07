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
  * `for_code_diff`: コンテナ実装を、それらの間のdiff行数をカウントするために処理したもの

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

TODO: 無視してよいwarningを列挙する。

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
* その実装にかかるコード差分が小さかったこと (図7, 8, 13, 15)
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

論文の図13は、B木実装においてcollective allocatorを使ってノードを再配置しpage-aware placementを実現する処理を示している。定義されている関数 `make_page_aware` は `include/far_memory_container/blocked/b_tree.ipp` の522行目にある `BTreeMap::batch_block` に、 `traverse` は531行目にある `BTreeMap::batch_block_step` に対応している。

図13中の主要な行は、実際の実装と下のように対応している。

|図13中の行番号|`b_tree.ipp` 内の行番号|
|-:|-:|
|2|527|
|3|528|
|6|533, 534, 535|
|7|539|
|8|540|
|10|541|
|12|542|
|13|544|

#### Figure 15

論文の図15は、B木実装においてcollective allocatorを使ってノードを再配置し、van Emde Boasレイアウトのpage-aware placementを実現する処理を示している。定義されている関数 `make_page_aware` は `include/far_memory_container/blocked/b_tree.ipp` の549行目にある `BTreeMap::batch_vEB` に、 `traverse` は564行目にある `BTreeMap::batch_vEB_step` に対応している。

図15中の主要な行は、実際の実装と下のように対応している。

|図15中の行番号|`b_tree.ipp` 内の行番号|
|-:|-:|
|2|560|
|3|561|
|6|566|
|8|570|
|9|571|
|11|572|
|13|573|
|14|575|
|17|580|
|19|581|
|21|589|
|22|590, 592|

#### Table 3

表3は `for_code_diff` ディレクトリ内にあるコンテナ実装間でdiffを取ることで再現できる。
このディレクトリにある実装は、 `include/far_memory_container` 内の実装に次の処理を加えたものである。

* `local` B-treeと `local+dfs` B-treeなど、実装を共有していたものを分離する
* `local` B-tree内のpage-aware placement用の処理など、分離の結果不要になった記述を削除する
* オブジェクト配置を一切コントロールしない、plain baseline variantを用意する
* 本質的なコード差分にあたらないものがdiffに表れないように削除・共通化する
  * オブジェクト間リンクの分析のための記述を削除
  * 空行を削除
  * インクルードパス、名前空間名を共通化

この処理を施した結果、 `for_code_diff` 内の構成は下の表のようになった。

|container variant|implementation|
|:-|:-|
|plain baseline B-tree|`for_code_diff/b_tree/baseline/b_tree.hpp` <br> `for_code_diff/b_tree/baseline/b_tree.ipp` <br> `for_code_diff/b_tree/baseline/b_tree_node.ipp` <br> `for_code_diff/b_tree/baseline/b_tree_iterator.ipp`|
|`hint` B-tree|`for_code_diff/b_tree/hint/b_tree.hpp` <br> `for_code_diff/b_tree/hint/b_tree.ipp` <br> `for_code_diff/b_tree/hint/b_tree_node.ipp` <br> `for_code_diff/b_tree/hint/b_tree_iterator.ipp`|
|`local` B-tree|`for_code_diff/b_tree/local/b_tree.hpp` <br> `for_code_diff/b_tree/local/b_tree.ipp` <br> `for_code_diff/b_tree/local/b_tree_node.ipp` <br> `for_code_diff/b_tree/local/b_tree_iterator.ipp`|
|`local+dfs` B-tree|`for_code_diff/b_tree/local_dfs/b_tree.hpp` <br> `for_code_diff/b_tree/local_dfs/b_tree.ipp` <br> `for_code_diff/b_tree/local_dfs/b_tree_node.ipp` <br> `for_code_diff/b_tree/local_dfs/b_tree_iterator.ipp`|
|`dfs` B-tree|`for_code_diff/b_tree/dfs/b_tree.hpp` <br> `for_code_diff/b_tree/dfs/b_tree.ipp` <br> `for_code_diff/b_tree/dfs/b_tree_node.ipp` <br> `for_code_diff/b_tree/dfs/b_tree_iterator.ipp`|
|`local+vEB` B-tree|`for_code_diff/b_tree/local_vEB/b_tree.hpp` <br> `for_code_diff/b_tree/local_vEB/b_tree.ipp` <br> `for_code_diff/b_tree/local_vEB/b_tree_node.ipp` <br> `for_code_diff/b_tree/local_vEB/b_tree_iterator.ipp`|
|`vEB` B-tree|`for_code_diff/b_tree/vEB/b_tree.hpp` <br> `for_code_diff/b_tree/vEB/b_tree.ipp` <br> `for_code_diff/b_tree/vEB/b_tree_node.ipp` <br> `for_code_diff/b_tree/vEB/b_tree_iterator.ipp`|
|plain baseline skip list|`for_code_diff/skip_list/baseline/skiplist.hpp`|
|`hint` skip list|`for_code_diff/skip_list/hint/skiplist.hpp`|
|`local` skip list|`for_code_diff/skip_list/local/skiplist.hpp`|
|`local+page` skip list|`for_code_diff/skip_list/local_page/skiplist.hpp`|
|`page` skip list|`for_code_diff/skip_list/page/skiplist.hpp`|

これを用いて、例えばplain baseline B-treeと `local+dfs` B-treeの差分は、下のコマンドを実行して得られる。

```bash
/workdir$ diff -rywW1 for_code_diff/b_tree/baseline/ for_code_diff/b_tree/local_dfs/ | sort | uniq -c
```

このコマンドは下の出力をする。

```
    571  
      7 <
    160 >
      1 diff -rywW1 for_code_diff/b_tree/baseline/b_tree.hpp for_code_diff/b_tree/local_dfs/b_tree.hpp
      1 diff -rywW1 for_code_diff/b_tree/baseline/b_tree.ipp for_code_diff/b_tree/local_dfs/b_tree.ipp
      1 diff -rywW1 for_code_diff/b_tree/baseline/b_tree_iterator.ipp for_code_diff/b_tree/local_dfs/b_tree_iterator.ipp
      1 diff -rywW1 for_code_diff/b_tree/baseline/b_tree_node.ipp for_code_diff/b_tree/local_dfs/b_tree_node.ipp
     19 |
```

出力のうち、 `>` の左にある数値160が `local+dfs` B-treeの実装で追加された行数、 `<` の左にある数値7が削除された行数、 `|` の左にある数値19が変更された行数である。
これは表3aの `local+dfs` の行に示された値と一致する。

`diff` コマンドの引数に指定するディレクトリパスを、上の表に従って変えることで、他の組み合わせについてもコード差分の行数を得ることができる。

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
TODO: 他の実行で出るべき値のあるファイルを用意し、説明。("Where appropriate, include descriptions of and links to files (included in the archive) that represent expected outputs (e.g., the log files expected to be generated by your tool on the given inputs)")

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

このスクリプトは、 `src` ディレクトリの下のソースファイルをコンパイルしたものを実行している。実行される処理の内訳については、[その1つ](TODO: link)に付されたコメントを参照されたい。

1回の実行時間の目安は1～2分である。これを短くしたい場合は、 `include/setting_basis.hpp` の19行目にある `NumElements` (コンテナに挿入する要素数)を小さくするのがいい。


### Reduction of Remote Swapping

> Using the collective allocator appropriately for object placement aware of the far-memory model actually benefits to reducing remote swapping.

このclaimをサポートするのは、[B木・スキップリストの各実装](#container-implementations)を用いて5.1.1章の"key-value store benchmark"を走らせた際の、データのスワップ量(図10, 11, 12)である。これは `scripts/kvs_benchmark.sh` を用いて再現できる。[Getting Started Guideの中でのベンチマーク実行](#testing-the-artifact)がその例である。

```bash
# reprint
/workdir$ ./scripts/kvs_benchmark.sh btree local+dfs 200 1.3 0.05
```

他の結果は `scripts/kvs_benchmark.sh` に与えるパラメータを変えることで再現される。当該スクリプトのパラメータの意味は下記の通りである。コマンド例の `btree` を `skiplist` に変えると図12の結果が得られるようになり、 `hint` を `local` など他のcontainer variantの名前に変えると対応する結果が得られる。 `200`, `1.3`, `0.05` はそれぞれベンチマーク・パラメータ $L$, $\alpha$, $U$ に対応している。

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

TODO: 各実行で出るべき値のあるファイルを用意し、説明。("Where appropriate, include descriptions of and links to files (included in the archive) that represent expected outputs (e.g., the log files expected to be generated by your tool on the given inputs)")

このスクリプトは、 `src` ディレクトリの下のソースファイルをコンパイルしたものを実行している。実行される処理の内訳については、[その1つ](TODO: link)に付されたコメントを参照されたい。

1回の実行時間の目安は1～2分である。これを短くしたい場合は、 `include/setting_basis.hpp` の19行目にある `NumElements` (コンテナに挿入する要素数)を小さくするのがいい。


## When You Create Your Own Container

collective allocatorを用いてコンテナを新たに実装する方法を、シンプルな連結リストの実装を例に説明する。
なお簡単のため、アロケータ抽象化のうちメモリ確保・解放以外のコンポーネントは無視する。

ここでは、次のようなインターフェイスを持った連結リストコンテナ `LinkedList` を実装する。

```cpp
template <class T, class Alloc> struct LinkedList {
  struct Node { Node *next, *prev; T value; };

  /* `header->next`: same as `header` if empty, a pointer to the first element otherwise
     `header->prev`: same as `header` if empty, a pointer to the last element otherwise */
  Node* header;

  Node* insert(Node* after_this_node, const T& elem);
  void erase(Node* node);
};
```

`LinkedList` の実装は `include/linked_list.hpp` に保存するとすると、これを利用するプログラムは次のようになる。

```cpp
#include "linked_list.hpp"
#include <farmalloc/collective_allocator.hpp>
#include <farmalloc/local_memory_store.hpp>
using namespace FarMalloc;

int main() {
  LinkedList<int, CollectiveAllocator<int, /* size of one page */ 4096>> list;

  /* ... use `list` ... */
}
```

このプログラムを `src/use_linked_list.hpp` に保存し、 `CMakeLists.txt` の末尾に次のように追記すれば、[ビルド](#build)時に実行可能バイナリ `/build/use_linked_list` が生成されるようになる。

```cmake
add_executable(use_linked_list
  src/use_linked_list.cpp
)
target_link_libraries(use_linked_list PRIVATE
  farmalloc_impl
)
```


### Simply Use a Collective Allocator

まず、 `Node` 型を確保するcollective allocatorオブジェクトをメンバに加える。

```cpp
template <class T, class Alloc> struct LinkedList {
  /* struct Node { ... }; */

  using NodeAlloc = typename Alloc::template rebind<Node>::other;
  NodeAlloc node_alloc;

  using SubAlloc = typename NodeAlloc::suballocator;

  /* ... */
};
```

メモリ確保・解放に `node_alloc` を使っていく。
メモリ確保については、とりあえずは、swappable plain sub-allocatorを使うこととする。

```cpp
#include <farmalloc/collective_allocator_traits.hpp>
using namespace FarMalloc;

template <class T, class Alloc> struct LinkedList {
  /* ... */

  Node* header = [this]{
    SubAlloc suballoc = node_alloc.get_suballocator(swappable_plain);
    Node* new_n = suballoc.allocate(1);  // allocate a 1-node region
    new (new_n) Node{.next = new_n, .prev = new_n};
    return new_n;
  }();

  Node* insert(Node* after_this_node, const T& elem) {
    SubAlloc suballoc = node_alloc.get_suballocator(swappable_plain);
    Node* new_n = suballoc.allocate(1);  // allocate a 1-node region
    new (new_n) Node{.next = after_this_node->next, .prev = after_this_node, .value = elem};
    after_this_node->next = after_this_node->next->prev = new_n;
    return new_n;
  }
  void erase(Node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node_alloc.deallocate(node, 1);  // deallocate a 1-node region
  }
};
```

メモリ解放については `node_alloc` を直接使えばよい。sub-allocatorの選択は内部で適切に行われる。

```cpp
template <class T, class Alloc> struct LinkedList {
  /* ... */

  void erase(Node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node_alloc.deallocate(node, 1);  // deallocate a 1-node region
  }
};
```

### Purely-Local Aware Placement

前のsubsectionで作った連結リストは、オブジェクト配置のremote swapping削減のための制御を全くしていない。
このsubsectionでは、purely-local aware placement (2.2, 4.3章) を実装する。
具体的には、次の優先度でpurely-local領域を使うようにする。

* `header` を最優先
* 他のノードは、先頭に近いほど優先

そのためにまず、purely-local領域にあるノードのうち最も優先度の低いものを `least_priority` というメンバ変数で追跡するようにする。
また、 `header` の定義でpurely-local領域の確保を試すように変更する。

```cpp
template <class T, class Alloc> struct LinkedList {
  /* ... */

  Node* least_priority = nullptr;
  Node* header = [this]{
    SubAlloc suballoc = node_alloc.get_suballocator(purely_local);
    Node* new_n = nullptr;
    try {
      new_n = suballoc.allocate(1);
      least_priority = new_n;
    } catch (...) {
      suballoc = node_alloc.get_suballocator(swappable_plain);
      new_n = suballoc.allocate(1);
    }
    new (new_n) Node{.next = new_n, .prev = new_n};
    return new_n;
  }();
};
```

`insert` の記述は論文の図7に近い。新たに挿入されるノードのメモリ確保を、1つ前のノードと同じsub-allocatorを用いて試行するようにする。失敗した場合、purely-local領域がフルになっているから、優先度に基づいて、ノードを1つswappable領域に再配置したり、メモリをswappable領域から確保しなおしたりする。

```cpp
template <class T, class Alloc> struct LinkedList {
  /* ... */

  Node* insert(Node* after_this_node, const T& elem) {
    SubAlloc suballoc = node_alloc.get_suballocator(after_this_node);
    Node* new_n;
    try {
      new_n = suballoc.allocate(1);
      // update least priority purely-local node if necessary
      if (least_priority == after_this_node) {
        least_priority = new_n;
      }
    } catch (...) { // `after_this_node` is in the purely-local region but the region is full
      if (least_priority != after_this_node) {
        // relocate the least priority purely-local node to the swappable region
        SubAlloc swappable = node_alloc.get_suballocator(swappable_plain);
        Node* relocated_to = swappable.allocate(1);
        new (relocated_to) Node{std::move(*least_priority)};
        relocated_to->prev->next = relocated_to->next->prev = relocated_to;
        least_priority->~Node();

        // reuse a 1-node region in the purely-local region
        new_n = least_priority;
        // update least priority purely-local node if necessary
        if (least_priority->prev != after_this_node) {
          least_priority = least_priority->prev;
        }
      } else {
        suballoc = node_alloc.get_suballocator(swappable_plain);
        new_n = suballoc.allocate(1);
      }
    }

    new (new_n) Node{.next = after_this_node->next, .prev = after_this_node, .value = elem};
    after_this_node->next = after_this_node->next->prev = new_n;
    return new_n;
  }
};
```

`erase` では、purely-local領域内のノードを削除したとき、swappable領域内のノードが存在していれば、そのなかで最も優先度の高いものをpurely-local領域に再配置する。

```cpp
template <class T, class Alloc> struct LinkedList {
  /* ... */

  void erase(Node* node) {
    SubAlloc purelylocal = node_alloc.get_suballocator(purely_local);
    bool need_relocation = (purelylocal.contains(node) && least_priority != header->prev);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node_alloc.deallocate(node, 1);

    if (need_relocation) {
      Node* relocated_to = purelylocal.allocate(1);
      new (relocated_to) Node{std::move(*least_priority->next)};
      relocated_to->prev->next = relocated_to->next->prev = relocated_to;
      least_priority->next->~Node();
      node_alloc.deallocate(least_priority->next, 1);

      least_priority = relocated_to;
    }
  }
};
```


### Purely-Local and Page-Aware Placement

最後に、このsubsectionでは、purely-local and page-aware placement (2.2, 4.4章) を実装する。
具体的には、論文の図13のように、全ノードを走査しながらswappable領域内のノードを再配置する。
走査順はリスト内の順序とする。

```cpp
template <class T, class Alloc> struct LinkedList {
  /* ... */

  void make_page_aware() {
    SubAlloc page = node_alloc.get_suballocator(new_per_page);
    SubAlloc purelylocal = node_alloc.get_suballocator(purely_local);

    Node* node = header;
    do {
      if (!purelylocal.contains(node)) {
        if (!page.is_occupancy_under(0.7)) {
          page = node_alloc.get_suballocator(new_per_page);
        }
        Node* relocated_to = page.allocate(1);
        new (relocated_to) Node{std::move(*node)};
        relocated_to->prev->next = relocated_to->next->prev = relocated_to;
        node->~Node();
        node_alloc.deallocate(node, 1);
        node = relocated_to;
      }

      node = node->next;
    } while (node != header);
  }
};
```

`insert` は、論文の図8のように、一杯のページからメモリ確保をしようとして失敗するケースをケアする。

```cpp
template <class T, class Alloc> struct LinkedList {
  /* ... */

  Node* insert(Node* after_this_node, const T& elem) {
    SubAlloc suballoc = node_alloc.get_suballocator(after_this_node);
    Node* new_n;
    try {
      new_n = suballoc.allocate(1);
      /* ... */
    } catch (...) {
      SubAlloc purelylocal = node_alloc.get_suballocator(purely_local);
      if (least_priority != after_this_node && purelylocal.contains(after_this_node)) {
        // relocate a node only when an allocation within the purely-local region fails

        /* ... */
      } else {
        suballoc = node_alloc.get_suballocator(swappable_plain);
        new_n = suballoc.allocate(1);
      }
    }
    /* ... */
  }
};
```

## List of Errata of the Paper

* 5.1.1章にて、"key-value store benchmark"の一種として*Read benchmark*を定義しているが、これは不要な定義である
* Figure 10, 11の凡例にて:
  * `hint-only` -> `hint`
  * `purely-local aware` -> `local`
  * `page-aware (dfs)` -> `dfs`
  * `purely-local and page-aware (dfs)` -> `local+dfs`
  * `page-aware (vEB)` -> `vEB`
  * `purely-local and page-aware (vEB)` -> `local+vEB`
* Figure 16のcaptionにて、 "skip list variants" -> "B-tree variants"
