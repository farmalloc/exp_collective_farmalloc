#!/bin/bash
cd $(dirname $0)/..


function exit_with_help {
    cat <<__EOS__ >&2
Usage: $(basename $0) STRUCTURE OBJ_PLMT

Parameters
    STRUCTURE:  one of {btree, skiplist}
    OBJ_PLMT:   when STRUCTURE is btree, one of {hint, local, local+dfs, dfs,
                                                 local+veb, veb}
                when STRUCTURE is skiplist, one of {hint, local, local+page,
                                                    page}
__EOS__
    exit 1
}


##########
# argument handling
##########
if [[ $# -ne 2 ]]; then
    exit_with_help
fi

total_data_size=$((160 * 13421773))
local_memory_cap=$((${total_data_size} * 50 / 100))
purely_local_cap_if_used=$((${local_memory_cap} - ${local_memory_cap} / 4096 / 2 * 4096))

structure=""
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
if [[ $structure = "b_tree" ]]; then
    case "$2" in
        hint)       obj_plmt="hinted";                      exec_args="";;
        local)      obj_plmt="collective_allocator_aware";  exec_args="$purely_local_cap_if_used 0";;
        local+dfs)  obj_plmt="collective_allocator_aware";  exec_args="$purely_local_cap_if_used 1";;
        dfs)        obj_plmt="page_aware";                  exec_args="1";;
        local+veb)  obj_plmt="collective_allocator_aware";  exec_args="$purely_local_cap_if_used 2";;
        veb)        obj_plmt="page_aware";                  exec_args="2";;
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
        hint)       obj_plmt="hinted";                      exec_args="";;
        local)      obj_plmt="collective_allocator_aware";  exec_args="$purely_local_cap_if_used 0";;
        local+page) obj_plmt="collective_allocator_aware";  exec_args="$purely_local_cap_if_used 1";;
        page)       obj_plmt="page_aware";                  exec_args="";;
        *)
            cat <<__EOS__ >&2
error: undefined OBJ_PLMT

__EOS__
            exit_with_help
            ;;
    esac
fi


##########
# run the benchmark
##########
log_name="logs/analyze_edges_of_$2_$1.log"
mkdir -p logs

echo "###analyze_edges_of_$2_$1###" | tee -a ${log_name}
./build/analyze_edges_of_${obj_plmt}_${structure} $exec_args | tee -a ${log_name}
