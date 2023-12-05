#include "setting_basis.hpp"

#include <far_memory_container/baseline/b_tree.hpp>
#include <farmalloc/hint_allocator.hpp>
#include <farmalloc/page_size.hpp>

#include <cstdlib>
#include <iostream>
#include <random>


int main()
{
    constexpr size_t purely_local_capacity = 0;
    constexpr bool batch_blocking = true;

    using namespace FarMemoryContainer::Baseline;
    using namespace FarMalloc;

    using Alloc = FarMalloc::HintAllocator<ValueType, PageSize>;
    BTreeMap<Key, Mapped, 2, std::less<Key>, Alloc> map{};
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
