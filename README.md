Artifact for "Collective Allocator Abstraction to Control Object Spatial Locality in C++"
===

* Artifact submission for #11 "Collective Allocator Abstraction to Control Object Spatial Locality in C++", which has been accepted to Programming 8.3.
  * authors: Takato Hideshima, Shigeyuki Sato, Tomoharu Ugawa
* URL to artifact: TODO
  * sha256 hash: `TODO`

This document contains
  * [Getting started guide](#getting-started-guide)
  * [List of claims](#overview-of-claims)
  * [Step-by-step instructions to reproduce the results in the paper](#step-by-step-instructions)
  * [Short tutorial to use the artifact](#when-you-create-your-own-container)


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
<summary>How to try without Docker</summary>

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
cd /workdir
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
# to use Makefile: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build/
```

TODO: 無視してよいwarningを列挙する。

### Testing the Artifact

Let's execute a benchmark program of a B-tree using our collective allocator library.
Here, we use the `local+dfs` variant of the B-tree (see Section 5.2) and
the "key-value store benchmark" in Section 5.1.1 with parameterers $L=200$, $\alpha=1.3$ and $U=0.05$.
This reproduces the result corresponding to the right most dark gray (purely-local & page-aware (dfs))<!-- TODO: errataに応じてlocal+dfsにする？ -->
plot in Figure 10c in the paper.

```bash
cd /workdir
scripts/kvs_benchmark.sh btree local+dfs 200 1.3 0.05
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

Our main claims are the followings, as described at the beginning of Section 5 in the paper.

1.  Our collective allocator abstraction is sufficiently expressive to implement various object placement strategies in a modular manner.
    * step-by-step instructions -> [Expressiveness](#expressiveness)
2.  Using the collective allocator appropriately for object placement aware of the far-memory model actually benefits to reducing remote swapping.
    * step-by-step instructions -> [Reduction of Remote Swapping](#reduction-of-remote-swapping)


## Step-by-Step Instructions
### Expressiveness

Our claim regarding expressiveness is:
> Our collective allocator abstraction is sufficiently expressive to implement various object placement strategies in a modular manner.

This is supported by the following facts:

  * **Container implementations**: we successfully implemented B-trees and skip lists with various object placement strategies using the collective allocator with small modifications on ordinary implementatinos of them. (examples in Section 4 and evaluation in Section 5.2)
  * **Cross-page link analysis**: these implementations exhibited expected behaviours in the cross-page link analysis (Section 5.1.2) with respect to object placement strategies. (Section 5.3)

Following subsections describes how to confirm these facts.

#### Container Implementations

The implementations of the B-tree and the skip list are placed in
the `include/far_memory_container` directory. Correspondence between
the container variant names in the paper (listed in Section 5.2) and 
source files are summarised in the following table.

|container variant|implementation|
|:-|:-|
|`hint` B-tree|`include/far_memory_container/baseline/b_tree.hpp` <br> `include/far_memory_container/baseline/b_tree.ipp` <br> `include/far_memory_container/baseline/b_tree_node.ipp` <br> `include/far_memory_container/baseline/b_tree_iterator.ipp`|
|`local` B-tree <br> `local+dfs` B-tree <br> `local+vEB` B-tree|`include/far_memory_container/blocked/b_tree.hpp` <br> `include/far_memory_container/blocked/b_tree.ipp` <br> `include/far_memory_container/blocked/b_tree_node.ipp` <br> `include/far_memory_container/blocked/b_tree_iterator.ipp`|
|`dfs` B-tree <br> `vEB` B-tree|`include/far_memory_container/page_aware/b_tree.hpp` <br> `include/far_memory_container/page_aware/b_tree.ipp` <br> `include/far_memory_container/page_aware/b_tree_node.ipp` <br> `include/far_memory_container/page_aware/b_tree_iterator.ipp`|
|`hint` skip list|`include/far_memory_container/baseline/skiplist.hpp`|
|`local` skip list <br> `local+page` skip list|`include/far_memory_container/blocked/skiplist.hpp`|
|`page` skip list|`include/far_memory_container/page_aware/skiplist.hpp`|

Note that `hint` B-tree and `hint` skip list do not use collective allocator. They used the standard C++ allocator.

##### Correspondence between examples in Section 4 and source code of implementations

In Section 4 in the paper, we picked `local+dfs` B-tree as an example to
demonstrate how to use our collective allocator using Figure 7, 8, and 13.
Figure 15 also demonstrates it using `local+vEB` B-tree as an example.
The functions in the figures correspond to the functions in `include/far_memory_container/blocked/b_tree.ipp`.

|figure|function in figure|function in `b_tree.ipp`|
|:-|:-|:-|
|7 & 8 | `BTree::ins_rec` | `BTreeMap::insert_step` |
|13    | `BTree::make_page_aware` | `BTreeMap::batch_block` |
|13    | `BTree::traverse` | `BTreeMap::batch_block_step` |
|15    | `BTree::make_page_aware` | `BTreeMap::batch_vEB` |
|15    | `BTree::traverse` | `BTreeMap::batch_vEB_step` |

<details>
<summary>Line-by-line correspondence</summary>

Line-by-line correspondence for the significant lines are listed in the following table.

|Figure #|line in figure|line in `b_tree.ipp`|
|-:|-:|-:|
|7|18|232|
|7|20, 21|254, 255|
|7|22|233|
|7|27|237, 326, 328|
|7|29|247|
|7|31, 32|254, 255, 327|
|7|34|245|
|7|35|247|
|7|40|258|
|7|41|259|
|8|8|197, 206|
|8|15, 16, 17|198, 234|
|13|2|527|
|13|3|528|
|13|6|533, 534, 535|
|13|7|539|
|13|8|540|
|13|10|541|
|13|12|542|
|13|13|544|
|15|2|560|
|15|3|561|
|15|6|566|
|15|8|570|
|15|9|571|
|15|11|572|
|15|13|573|
|15|14|575|
|15|17|580|
|15|19|581|
|15|21|589|
|15|22|590, 592|

</details>

##### Amount of code differences between baseline and containers using the collective allocator (Table 3 in the paper)

In Table 3 in the paper, we counted the number of different lines between the baselines
and the containers using the collective allocator using the `diff` command. Here the 
baselines are the containers that are similar to `hint` B-tree and `hint` skip list,
but allocation hint parameters are not given to the allocator.

As we showed in the previous table, a single set of source files implements multiple
variants of containers.  For example, `local` B-tree, `local+dfs` B-tree, and `local-vEB`
B-tree are implemented in the same source file. Thus, we separated implementations so
that each separated implementation contains
only the variant specific code and common code used by the variant.
Separate implementations are stored in the `for_code_diff` directory.

Amount of different lines (Table 3 in the paper) are counted by compareing with
the files for the baseline and those for the separated implementation.
For `local+dfs` B-tree, for example, the following command gives the amount of
differences.

```bash
cd /workdir/for_code_diff
diff -rywW1 b_tree/baseline b_tree/local_dfs | sort | uniq -c
```

Following expected output shows there are 7 dels (`<`), 160 adds (`>`), and 19 modifies (`|`).
These numbers match those of `local+dfs` in Table 3a.

```
    571  
      7 <
    160 >
      1 diff -rywW1 b_tree/baseline/b_tree.hpp b_tree/local_dfs/b_tree.hpp
      1 diff -rywW1 b_tree/baseline/b_tree.ipp b_tree/local_dfs/b_tree.ipp
      1 diff -rywW1 b_tree/baseline/b_tree_iterator.ipp b_tree/local_dfs/b_tree_iterator.ipp
      1 diff -rywW1 b_tree/baseline/b_tree_node.ipp b_tree/local_dfs/b_tree_node.ipp
     19 |
```

Directories for separate implementations are summarised
in the following table. Note that baselines are created from `hint` containers
by removing allocation hint parameters.  Also note that inessential diffrences
such as those in measruement code and include paths are eliminated from
the separate implementation.

|container variant|directory|
|:-|:-|
|baseline B-tree|`for_code_diff/b_tree/baseline`|
|`hint` B-tree|`for_code_diff/b_tree/hint`|
|`local` B-tree|`for_code_diff/b_tree/local`|
|`local+dfs` B-tree|`for_code_diff/b_tree/local_dfs`|
|`dfs` B-tree|`for_code_diff/b_tree/dfs`|
|`local+vEB` B-tree|`for_code_diff/b_tree/local_vEB`|
|`vEB` B-tree|`for_code_diff/b_tree/vEB`|
|baseline skip list|`for_code_diff/skip_list/baseline`|
|`hint` skip list|`for_code_diff/skip_list/hint`|
|`local` skip list|`for_code_diff/skip_list/local`|
|`local+page` skip list|`for_code_diff/skip_list/local_page`|
|`page` skip list|`for_code_diff/skip_list/page`|

#### Cross-page link analysis (Figure 9 in Section 5.2)

The files `src/analyze_edges_*.cpp` are benchmark drivers for the corss-page link analysis.
Though the [build](#build) section in the Getting Started Guide, they have already compiled
into the `build` directory.

They can be executed using the `scripts/analyze_edges.sh` script. The following
command counts the number of edges for each edge type for the `hint` B-tree,
which corresponds to `hint` in Figure 9a.

```bash
cd /workdir
scripts/analyze_edges.sh btree hint
```
The expected output looks like the following. The last thee numbers
are the numbers of purely-local  in-page, and cross-page links,
from left to right, respectively.
(TODO: 数字がおかしい?)
They are 5592382, 0, and 4473384 in this example. Figure 9 in the paper plots
the ratios of these numbers.

```
###analyze_edges_of_local+dfs_btree###
#NumElements    PurelyLocalCapacity[B]  batch_blocking  construction_duration[ns]       purely_local_edges      same_page_edges diff_pages_edges
13421773        2147483712      1       85143447311     5592382 0       4473384
```

Numbers for other variants can be obtained by giving appropriate parameters to the script.
The usage of the script is:

```
analyze_edges.sh structure placement
```

where
  * `structure` is either `btree` or `skiplist` and
  * `placement` is one of the lowercase labels bars in Figure 9, such as `dfs`, `veb`, or `local+dfs`.

The expected output for each variant is placed in the `TODO` directory of this artifact. **TODO**

Note that a single execution of ``analyze_edges.sh`` will complete in a few minutes.
Giving a smaller number to `NumElements` in `include/setting_basis.hpp:19` will
reduce the execution time.

##### Execute all with a single command

The `TODO` script executes the benchmark program for all container variants, and
the `TODO` script reproduce the charts in Figure 9.

```
cd /workdir
scripts/execute all TODO TODO
scripts/generate figure 9 TODO TODO
```

### Reduction of Remote Swapping

Our claim regarding reduction of remote swapping is:

> Using the collective allocator appropriately for object placement aware of the far-memory model actually benefits to reducing remote swapping.

This claim is supported by the experimentation in Section 5.3, where we
executed the benchmark programs and counted the number of swapping.

The remaining files in the `src` directory (those that do not start with `analyze_edges`)
are benchmark drivers for this experimentation.  Though the [build](#build)
section in the Getting Started Guide, they have already compiled into the `build` directory.

The benchmark programs can be executed though `scripts/kvs_benchmark.sh`. A single execution
gives a number for a single data point in Figure 10, 11, and 12. We have already
tried it in [Testing the Artifact](#testing-the-artifact) section in the Getting Started
Guide.

For short, the following command will give the number of swapping for the
`local+dfs` variant of the B-tree with $L=200$, $\alpha=1.3$ and $U=0.05$,
corresponding to the right most dark gray (purely-local & page-aware (dfs))<!-- TODO: errataに応じてlocal+dfsにする？ -->
plot in Figure 10c in the paper.

```bash
cd /workdir
scripts/kvs_benchmark.sh btree local+dfs 200 1.3 0.05
```

The usage of this script is

```
Usage: kvs_benchmark.sh structure placement local-capacity skewness update-ratio
```

where
  * `structure` is either `btree` or `skiplist`
  * `placement` is the variant name; one of `hint`, `local`, `local+dfs`, `dfs`, `local+veb`, `veb`, `local+page`, `page`
  * `local-capacity`, $L$, is the capacity of local memory (percentage to the total data size used in the benchmark)
  * `skewness`, $\alpha$, is the skewness of the data (Zipfan skewness); either 0.8 or 1.3
  * `update-ratio`, $U$, is the fraction of update queries in the data; either 0.05 or 0.5.

The expected output for each (variant? execution? **TODO**) is placed in the `TODO` directory of this artifact. **TODO**

Note that a single execution of ``analyze_edges.sh`` will complete in a few minutes.
Giving a smaller number to `NumElements` in `include/setting_basis.hpp:19` will
reduce the execution time.

##### Execute all with a single command

The `TODO` script executes the benchmark program for all container variants with
all combinations of parameters to reproduce Figures 10, 11, and 12.
The `TODO` script reproduces these figures.

```
cd /workdir
scripts/execute all TODO TODO
scripts/generate figure 10, 11, 12 TODO TODO
```

**TODO**
Note that a single execution of `TODO` will complete in **TODO HOW LONG TIME?**.  
Giving a smaller number to `NumElements` in `include/setting_basis.hpp:19` will
reduce the execution time.
**TODO** For example, execution will complate in XX minutes if we give XX to
`NumElements`.  However, the results will be totally different from Figures 10, 11, and 12.

## Tutorial to Use Collective Allocator

This is the tutorial to develop a container using the collective allocator.
As the artifact is a library, we provide a short tutorial to write a program using the library.
You can skip this section if you just want to reproduce the results in the paper.

In this tutorial, we develop a linked list, `LinkedList`, with focusing on
memory allocation and deallocation. We create files in this artifact directory `/workdir`
in Docker container.

### Step 1. Empty Program

<small>
collective allocatorを用いてコンテナを新たに実装する方法を、シンプルな連結リストの実装を例に説明する。
なお簡単のため、アロケータ抽象化のうちメモリ確保・解放以外のコンポーネントは無視する。
</small>

First, we make an interface of `Linkedlist` in `include/linked_list.hpp`.
The interface is as follows.

<small>
ここでは、次のようなインターフェイスを持った連結リストコンテナ `LinkedList` を実装する。
</small>

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

The `LinkedList` container is expected to be used as follows.

<small>
`LinkedList` の実装は `include/linked_list.hpp` に保存するとすると、これを利用するプログラムは次のようになる。
</small>

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

Let's create `src/use_linked_list.hpp` **TODO: use_linked_list.cpp ?** containing
the empty program using the `LinkedList` above. Then, add the followings to `CMakeLists.txt`
in `/workdir` so that this file will be compiled.

<small>
このプログラムを `src/use_linked_list.hpp` に保存し、 `CMakeLists.txt` の末尾に次のように追記すれば、[ビルド](#build)時に実行可能バイナリ `/build/use_linked_list` が生成されるようになる。
</small>

```cmake
add_executable(use_linked_list
  src/use_linked_list.cpp
)
target_link_libraries(use_linked_list PRIVATE
  farmalloc_impl
)
```

Now, we can build `/build/use_linked_list`, which creates a `LinkedList` and does nothing.

```bash
cd /workdir
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
# to use Makefile: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build/
```

### Step 2. Allocation and Deallocation without Awareing Placement

First, we add an instance of the collective allocator `node_alloc` that allocates
for `Node` type to the `LinkedList` struct. We also define two type names, `NodeAlloc`
and `SubAlloc`. `SubAlloc` is the type of sub-allocator, which is characteristic of
the collective allocator.

Edit `include/linked_list.hpp` as follows (add three lines).

<small>
まず、 `Node` 型を確保するcollective allocatorオブジェクトをメンバに加える。
</small>

```cpp
template <class T, class Alloc> struct LinkedList {
  /* struct Node { ... }; */

  using NodeAlloc = typename Alloc::template rebind<Node>::other;
  NodeAlloc node_alloc;

  using SubAlloc = typename NodeAlloc::suballocator;

  /* ... */
};
```

Next, we implement the `insert` method of the `LinkedList` struct
as well as initialisation code for the `header` member. These implementations
use a sub-allocator of the collective allocator `node_alloc`
to allocate memory for `Node`.
To allocate memory for a node, we first get a sub-allocator by using
`get_suballocator` method. Then, allocate memory from the sub-allocator.
In this step,
we use the swappable plain sub-allocator, which owns unlimited capacity of
far-memory.

Add the following code to `include/linked_list.hpp`.

<small>
メモリ確保・解放に `node_alloc` を使っていく。
メモリ確保については、とりあえずは、swappable plain sub-allocatorを使うこととする。
</small>

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

  /* ... */
};
```

Finally, we implement the `erase` method, which deallocate a `Node`.
For deallocation, we can simply use the `deallocate` method of the collective allocator.

<small>
メモリ解放については `node_alloc` を直接使えばよい。sub-allocatorの選択は内部で適切に行われる。
</small>

```cpp
#include <farmalloc/collective_allocator_traits.hpp>
using namespace FarMalloc;

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

Now, we modify the `LinkedList` to use local memory.
This corresponds to the purely-local ware placement (Section 2.2 and 4.3) in the paper.

<small>
前のsubsectionで作った連結リストは、オブジェクト配置のremote swapping削減のための制御を全くしていない。
このsubsectionでは、purely-local aware placement (2.2, 4.3章) を実装する。
</small>

Our placement strategy here is to place as many nodes as possible in local memory,
giving higher priority to the closer node to the header.
To do so, we introduce a member `least_priority` to keep track of the least prioritized
node among nodes in local memory.

```cpp
template <class T, class Alloc> struct LinkedList {
  /* ... */

  Node* least_priority = nullptr;

  /* ... */
};
```

<small>
具体的には、次の優先度でpurely-local領域を使うようにする。

* `header` を最優先
* 他のノードは、先頭に近いほど優先

そのためにまず、purely-local領域にあるノードのうち最も優先度の低いものを `least_priority` というメンバ変数で追跡するようにする。
また、 `header` の定義でpurely-local領域の確保を試すように変更する。
</small>

In the initialisation code for `header`, we try to allocate from
the `purely_local` sub-allocator instead of the `swappable_plain` one.
The `allocate` method of the `purely_local` sub-allocator throws an
exception when local memory is full. In such a case, we allocate
memory from the `sappable_plain` sub-allocator.

When we successfully allocate memory from the `purely_local` sub-allocator,
we track it with `least_priority` as it is the only `Node` in local memory.

The header initialisation code is as follows.

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

In the implementation of the `insert` method, we first try to allocate
memory for a new `Node` from the same sub-allocator as the `Node`
immediately after which the new `Node` will be inserted.
The `after_this_node` parameter of `insert` is such a `Node`.

First, we take a sub-allocator allocated memory for `after_this_node`.
Then, we try to allocate from the sub-allocator. If it fails,
`after_this_node` is in local memory and local memory is full.


<small>
`insert` の記述は論文の図7に近い。新たに挿入されるノードのメモリ確保を、1つ前のノードと同じsub-allocatorを用いて試行するようにする。失敗した場合、purely-local領域がフルになっているから、優先度に基づいて、ノードを1つswappable領域に再配置したり、メモリをswappable領域から確保しなおしたりする。
</small>

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
