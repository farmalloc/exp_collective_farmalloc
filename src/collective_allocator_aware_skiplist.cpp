#include "setting.hpp"

#include <far_memory_container/blocked/skiplist.hpp>
#include <farmalloc/collective_allocator.hpp>
#include <farmalloc/local_memory_store.hpp>
#include <farmalloc/page_size.hpp>

#include <umap/umap.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>


int main(int argc, char* argv[])
{
    /***********
     * handling of command line arguments
     ***********/
    if (argc <= 2) {
        std::cerr << "usage: " << argv[0] << " purely_local_capacity(size_t) batch_blocking(0:None/1:DepthFirst)" << std::endl;
        std::quick_exit(EXIT_FAILURE);
    }

    const auto purely_local_capacity = [&] {
        size_t tmp;
        std::istringstream sstr{argv[1]};
        sstr >> tmp;
        if (sstr.fail()) {
            throw std::invalid_argument{std::string{argv[1]}};
        }
        return tmp;
    }();
    const bool batch_blocking = [&] {
        int tmp;
        std::istringstream sstr{argv[2]};
        sstr >> tmp;
        if (sstr.fail()) {
            throw std::invalid_argument{std::string{argv[2]}};
        }
        return static_cast<bool>(tmp);
    }();

    using namespace FarMemoryContainer::Blocked;
    using namespace FarMalloc;


    /***********
     * instantiation of a collective allocator and a container
     ***********/
    using Alloc = FarMalloc::CollectiveAllocator<ValueType, PageSize>;
    SkiplistMap<Key, Mapped, std::less<Key>, Alloc> map{Alloc{purely_local_capacity}};


    /***********
     * insertion of `NumElements` elements
     ***********/
    std::mt19937 prng;
    const auto cons_dur = construct(prng, map);


    /***********
     * batch rearrangement of nodes for page-aware placement
     ***********/
    if (batch_blocking) {
        map.batch_block();
    }


    /***********
     * enable remote swapping for the swappable region
     *
     * To shorten the experiment time, remote swapping is disabled
     * until just before the measurement section.
     * This function enables it, and sets all pages to be flushed.
     ***********/
    LocalMemoryStore::mode_change();
    /***********
     * reset swapping counters to zero
     *
     * The counters below are incremented each time a page swaps in/out.
     ***********/
    LocalMemoryStore::read_cnt = 0;
    LocalMemoryStore::write_cnt = 0;


    /***********
     * running "key-value store benchmark"
     ***********/
    const auto query_dur = range_query(prng, map);

    std::cout << "#NumElements\t"
              << "NIteration\t"
              << "ZipfSkewness\t"
              << "UpdateRatio\t"
              << "PurelyLocalCapacity[B]\t"
              << "UMAP_BUFSIZE[pages]\t"
              << "batch_blocking\t"
              << "construction_duration[ns]\t"
              << "query_duration[ns]\t"
              << "query_read_cnt\t"
              << "query_write_cnt" << std::endl;
    std::cout << NumElements << '\t'
              << NIteration << '\t'
              << ZipfSkewness << '\t'
              << UpdateRatio << "\t"
              << purely_local_capacity << "\t"
              << umapcfg_get_max_pages_in_buffer() << '\t'
              << static_cast<int>(batch_blocking) << '\t'
              << cons_dur.count() << '\t'
              << query_dur.count() << "\t"
              << LocalMemoryStore::read_cnt << '\t'
              << LocalMemoryStore::write_cnt << std::endl;

    std::quick_exit(EXIT_SUCCESS);
}
