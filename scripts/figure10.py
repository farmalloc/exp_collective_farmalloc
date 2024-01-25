#!/usr/bin/env python3
import os.path
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import rcParams, ticker
import numpy.lib.recfunctions as rfn
from cycler import cycler


def main():
    rcParams["font.family"] = "sans-serif"
    rcParams["font.sans-serif"] = ["IPAexGothic"]
    rcParams["pdf.fonttype"] = 42  # TrueType
    rcParams["font.size"] = 7  # footnotesize=8
    rcParams['lines.markersize'] = 3
    markers = ["o", "x", "v", "D", "s", "*", "^", "d"]
    plt.style.use('tableau-colorblind10')
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

    log_dir = os.path.join(os.path.dirname(__file__),
                           "../logs")

    dtype = [("NumElements", np.int64), ("NIteration", np.int64), ("ZipfSkewness", np.float64), ("UpdateRatio", np.float64),
             ("PurelyLocalCapacity", np.int64), ("UMAP_BUFSIZE",
                                                 np.int64), ("batch_blocking", np.int64),
             ("construction_duration", np.int64), ("query_duration", np.int64), ("query_read_cnt", np.int64), ("query_write_cnt", np.int64)]

    variants = {}
    variants["dfs"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_dfs_btree.log"), dtype=dtype, ndmin=2)
    variants["vEB"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_veb_btree.log"), dtype=dtype, ndmin=2)
    variants["local"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_local_btree.log"), dtype=dtype, ndmin=2)
    variants["local+dfs"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_local+dfs_btree.log"), dtype=dtype, ndmin=2)
    variants["local+vEB"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_local+veb_btree.log"), dtype=dtype, ndmin=2)
    variants["hint"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_hint_btree.log"), dtype=dtype, ndmin=2)

    for variant, color, marker in zip(variants.keys(), colors, markers):
        used_local_buf = (variants[variant]["PurelyLocalCapacity"] + variants[variant]["UMAP_BUFSIZE"] * 4096) \
                          * 100 / (160 * variants[variant]["NumElements"]) + 0.5
        query_swap_cnt = variants[variant]["query_read_cnt"] + variants[variant]["query_write_cnt"]
        tmp = rfn.append_fields(variants[variant],
                                ("used_local_buf", "query_swap_cnt"),
                                (used_local_buf, query_swap_cnt),
                                dtypes=(np.int64, np.int64))
        tmp.sort(order="used_local_buf")
        variants[variant] = dict(data=tmp, color=color, marker=marker)

    def create_figure(variants, subfigures, create_legend=False):
        subfig_suffix_iter = iter(subfigures)
        for zipf_skewness, update_ratio in ((0.8, 0.05), (0.8, 0.5), (1.3, 0.05), (1.3, 0.5)):
            fig = plt.figure(figsize=(2.55, 1.3))
            ax = fig.add_subplot(1, 1, 1)
            ax.set_xlabel(r"$L =$(local memory usage)/(data size) [%]")
            ax.set_ylabel(r"amount of swapped data")
            ax.yaxis.set_major_formatter(ticker.ScalarFormatter(useMathText=True))
            ax.ticklabel_format(style="sci", axis="y", scilimits=(0, 0))

            lines = []
            labels = []

            ax.axvline(100, color="black", linestyle="dashed")

            for label in variants:
                variant = variants[label]
                datum = variant["data"][
                            (variant["data"]["ZipfSkewness"] == zipf_skewness)
                            & (variant["data"]["UpdateRatio"] == update_ratio)
                        ]
                labels.append(label)
                lines.append(ax.plot(
                    datum["used_local_buf"],
                    datum["query_swap_cnt"],
                    color=variant["color"],
                    marker=variant["marker"]
                )[0])

            x_max = np.max([variants[k]["data"]["used_local_buf"][-1] for k in variants])
            ax.set_xlim((0, x_max))
            ax.set_xticks(tuple(x_max * i // 8 for i in range(9)))

            (bottom, top) = ax.get_ylim()
            ax.set_ylim((0.0, top))

            fig.savefig(
                os.path.join(
                    os.path.dirname(__file__), f"../charts/figure10{next(subfig_suffix_iter)}.pdf"
                ),
                bbox_inches="tight",
            )

            if create_legend:
                legendfig = plt.figure()
                legendfig.legend(lines, labels, ncol=len(lines)).get_frame().set_alpha(1.0)
                legendfig.savefig(
                    os.path.join(
                        os.path.dirname(__file__), f"../charts/figure10_legend.pdf"
                    ),
                    bbox_inches="tight",
                )

    create_figure(variants, "abcd", create_legend=True)
    create_figure({k: variants[k] for k in ["dfs", "local", "local+dfs", "hint"]}, subfigures="abcd")
    create_figure({k: variants[k] for k in ["dfs", "vEB", "local+dfs", "local+vEB"]}, subfigures="efgh")


if __name__ == "__main__":
    main()
