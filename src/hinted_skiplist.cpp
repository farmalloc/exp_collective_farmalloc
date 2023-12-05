#include "setting.hpp"

#include <far_memory_container/baseline/skiplist.hpp>
#include <farmalloc/hint_allocator.hpp>
#include <farmalloc/local_memory_store.hpp>
#include <farmalloc/page_size.hpp>

#include <umap/umap.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>


int main()
{
    constexpr size_t purely_local_capacity = 0;
    constexpr bool batch_blocking = true;

    using namespace FarMemoryContainer::Baseline;
    using namespace FarMalloc;

    using Alloc = FarMalloc::HintAllocator<ValueType, PageSize>;
    SkiplistMap<Key, Mapped, std::less<Key>, Alloc> map{};
    std::mt19937 prng;

    const auto cons_dur = construct(prng, map);

    if (batch_blocking) {
        map.batch_block();
    }

    LocalMemoryStore::mode_change();
    LocalMemoryStore::read_cnt = 0;
    LocalMemoryStore::write_cnt = 0;

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
