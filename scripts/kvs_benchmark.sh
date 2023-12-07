#!/bin/bash
cd $(dirname $0)/..


function exit_with_help {
    cat <<__EOS__ >&2
Usage: $(basename $0) STRUCTURE OBJ_PLMT LOCAL_MEM_CAP ZIPF_SKEWNESS UPDATE_RATIO

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
__EOS__
    exit 1
}


##########
# argument handling
##########
if [[ $# -ne 5 ]]; then
    exit_with_help
fi

if [[ ! "$3" =~ ^\+?[1-9][0-9]*$ ]]; then
    cat <<__EOS__ >&2
error: LOCAL_MEM_CAP should be an unsigned integer

__EOS__
    exit_with_help
fi

total_data_size=$((160 * 13421773))
local_memory_cap=$((${total_data_size} * $3 / 100))
local_memory_cap_in_pages=$((${local_memory_cap} / 4096))
purely_local_cap_if_used=$((${local_memory_cap} - ${local_memory_cap_in_pages} / 2 * 4096))

# structure=""
case "$1" in
    btree) structure="b_tree";;
    skiplist) structure="skiplist";;
    *)
        cat <<__EOS__ >&2
error: undefined STRUCTURE

__EOS__
        exit_with_help
        ;;
esac

obj_plmt=""
exec_args=""
swap_cache_size=0
if [[ $structure = "b_tree" ]]; then
    case "$2" in
        hint)       obj_plmt="hinted";                      swap_cache_size=$local_memory_cap_in_pages;             exec_args="";;
        local)      obj_plmt="collective_allocator_aware";  swap_cache_size=$(($local_memory_cap_in_pages / 2));    exec_args="$purely_local_cap_if_used 0";;
        local+dfs)  obj_plmt="collective_allocator_aware";  swap_cache_size=$(($local_memory_cap_in_pages / 2));    exec_args="$purely_local_cap_if_used 1";;
        dfs)        obj_plmt="page_aware";                  swap_cache_size=$local_memory_cap_in_pages;             exec_args="1";;
        local+veb)  obj_plmt="collective_allocator_aware";  swap_cache_size=$(($local_memory_cap_in_pages / 2));    exec_args="$purely_local_cap_if_used 2";;
        veb)        obj_plmt="page_aware";                  swap_cache_size=$local_memory_cap_in_pages;             exec_args="2";;
        *)
            cat <<__EOS__ >&2
error: undefined OBJ_PLMT

__EOS__
            exit_with_help
            ;;
    esac
fi
if [[ $structure = "skiplist" ]]; then
    case "$2" in
        hint)       obj_plmt="hinted";                      swap_cache_size=$local_memory_cap_in_pages;             exec_args="";;
        local)      obj_plmt="collective_allocator_aware";  swap_cache_size=$(($local_memory_cap_in_pages / 2));    exec_args="$purely_local_cap_if_used 0";;
        local+page) obj_plmt="collective_allocator_aware";  swap_cache_size=$(($local_memory_cap_in_pages / 2));    exec_args="$purely_local_cap_if_used 1";;
        page)       obj_plmt="page_aware";                  swap_cache_size=$local_memory_cap_in_pages;             exec_args="";;
        *)
            cat <<__EOS__ >&2
error: undefined OBJ_PLMT

__EOS__
            exit_with_help
            ;;
    esac
fi

if [[ ! ( "$4" = 0.8 || "$4" = 1.3 )]]; then
            cat <<__EOS__ >&2
error: wrong ZIPF_SKEWNESS

__EOS__
            exit_with_help
fi
if [[ ! ( "$5" = 0.05 || "$5" = 0.5 )]]; then
            cat <<__EOS__ >&2
error: wrong UPDATE_RATIO

__EOS__
            exit_with_help
fi


##########
# run the benchmark
##########
log_name="logs/analyze_edges_of_$2_$1.log"
mkdir -p logs
echo "log file: ${log_name}"

echo "###analyze_edges_of_$2_$1###" | tee -a ${log_name}
export UMAP_LOG_LEVEL=ERROR
export UMAP_BUFSIZE=$swap_cache_size
./build/${obj_plmt}_${structure}_skew${4}_update${5} $exec_args | tee -a ${log_name}
