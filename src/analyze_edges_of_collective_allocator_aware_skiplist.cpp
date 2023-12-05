#include "setting_basis.hpp"

#include <far_memory_container/blocked/skiplist.hpp>
#include <farmalloc/collective_allocator.hpp>
#include <farmalloc/page_size.hpp>

#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>


int main(int argc, char* argv[])
{
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

    using Alloc = FarMalloc::CollectiveAllocator<ValueType, PageSize>;
    SkiplistMap<Key, Mapped, std::less<Key>, Alloc> map{Alloc{purely_local_capacity}};
    std::mt19937 prng;

    const auto cons_dur = construct(prng, map);

    if (batch_blocking) {
        map.batch_block();
    }

    auto [purely_local_edges, same_page_edges, diff_pages_edges] = map.analyze_edges<PageSize>();

    std::cout << "#NumElements\t"
              << "PurelyLocalCapacity[B]\t"
              << "batch_blocking\t"
              << "construction_duration[ns]\t"
              << "purely_local_edges\t"
              << "same_page_edges\t"
              << "diff_pages_edges" << std::endl;
    std::cout << NumElements << '\t'
              << purely_local_capacity << "\t"
              << batch_blocking << '\t'
              << cons_dur.count() << '\t'
              << purely_local_edges << '\t'
              << same_page_edges << '\t'
              << diff_pages_edges << std::endl;

    std::quick_exit(EXIT_SUCCESS);
}
