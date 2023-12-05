#include "setting.hpp"

#include <far_memory_container/blocked/b_tree.hpp>
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
    if (argc <= 2) {
        std::cerr << "usage: " << argv[0] << " purely_local_capacity(size_t) batch_blocking(0:None/1:DepthFirst/2:vEB)" << std::endl;
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
    const auto batch_blocking = [&] {
        int tmp;
        std::istringstream sstr{argv[2]};
        sstr >> tmp;
        if (sstr.fail()) {
            throw std::invalid_argument{std::string{argv[2]}};
        }
        return static_cast<BlockingMode>(tmp);
    }();

    using namespace FarMemoryContainer::Blocked;
    using namespace FarMalloc;

    using Alloc = FarMalloc::CollectiveAllocator<ValueType, PageSize>;
    BTreeMap<Key, Mapped, 2, std::less<Key>, Alloc> map{Alloc{purely_local_capacity}};
    std::mt19937 prng;

    const auto cons_dur = construct(prng, map);

    switch (batch_blocking) {
    case DepthFirst:
        map.batch_block();
        break;

    case vEB:
        map.batch_vEB();
        break;

    case None:
    default:
        break;
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
