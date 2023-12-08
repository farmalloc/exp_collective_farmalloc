#include "setting_basis.hpp"

#include <far_memory_container/page_aware/b_tree.hpp>
#include <farmalloc/collective_allocator.hpp>
#include <farmalloc/page_size.hpp>

#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>


int main(int argc, char* argv[])
{
    /***********
     * handling of command line arguments
     ***********/
    if (argc <= 1) {
        std::cerr << "usage: " << argv[0] << " batch_blocking(0:None/1:DepthFirst/2:vEB)" << std::endl;
        std::quick_exit(EXIT_FAILURE);
    }

    constexpr size_t purely_local_capacity = 0;
    const auto batch_blocking = [&] {
        int tmp;
        std::istringstream sstr{argv[1]};
        sstr >> tmp;
        if (sstr.fail()) {
            throw std::invalid_argument{std::string{argv[1]}};
        }
        return static_cast<BlockingMode>(tmp);
    }();


    /***********
     * instantiation of a collective allocator and a container
     ***********/
    using namespace FarMemoryContainer::PageAware;
    using namespace FarMalloc;

    using Alloc = FarMalloc::CollectiveAllocator<ValueType, PageSize>;
    BTreeMap<Key, Mapped, 2, std::less<Key>, Alloc> map{Alloc{purely_local_capacity}};


    /***********
     * insertion of `NumElements` elements
     ***********/
    std::mt19937 prng;
    const auto cons_dur = construct(prng, map);


    /***********
     * batch rearrangement of nodes for page-aware placement
     ***********/
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


    /***********
     * calling the method of analysis of links between objects
     *
     * `PageSize` is passed because container implementations doesn't know that
     ***********/
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
