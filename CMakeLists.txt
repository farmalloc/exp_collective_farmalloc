cmake_minimum_required(VERSION 3.13)

project(exp_collective_farmalloc)


add_subdirectory(library)


add_library(unoptimized_read OBJECT src/read.cpp)
target_link_libraries(unoptimized_read PRIVATE farmalloc_compile_ops)
target_include_directories(unoptimized_read PRIVATE include/)


foreach(STRUCTURE IN ITEMS b_tree skiplist)
  foreach(OBJ_PLMT IN ITEMS hinted page_aware collective_allocator_aware)
    add_executable(analyze_edges_of_${OBJ_PLMT}_${STRUCTURE}
      src/analyze_edges_of_${OBJ_PLMT}_${STRUCTURE}.cpp
    )
    target_link_libraries(analyze_edges_of_${OBJ_PLMT}_${STRUCTURE} PRIVATE
      farmalloc_compile_ops
      farmalloc_abst
      farmalloc_impl
      util
    )
    target_include_directories(analyze_edges_of_${OBJ_PLMT}_${STRUCTURE} PRIVATE include/)

    foreach(ZIPF_SKEWNESS IN ITEMS 0.8 1.3)
      foreach(UPDATE_RATIO IN ITEMS 0.05 0.5)
        add_executable(${OBJ_PLMT}_${STRUCTURE}_skew${ZIPF_SKEWNESS}_update${UPDATE_RATIO}
          src/${OBJ_PLMT}_${STRUCTURE}.cpp
        )
        target_link_libraries(${OBJ_PLMT}_${STRUCTURE}_skew${ZIPF_SKEWNESS}_update${UPDATE_RATIO} PRIVATE
          farmalloc_compile_ops
          unoptimized_read
          farmalloc_abst
          farmalloc_impl
          util
          umap
        )
        target_compile_definitions(${OBJ_PLMT}_${STRUCTURE}_skew${ZIPF_SKEWNESS}_update${UPDATE_RATIO} PRIVATE
          Zipf_skewness=${ZIPF_SKEWNESS}
          Update_ratio=${UPDATE_RATIO}
        )
        target_include_directories(${OBJ_PLMT}_${STRUCTURE}_skew${ZIPF_SKEWNESS}_update${UPDATE_RATIO} PRIVATE include/)
      endforeach(UPDATE_RATIO)
    endforeach(ZIPF_SKEWNESS)
  endforeach(OBJ_PLMT)
endforeach(STRUCTURE)
