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
    plt.rcParams['axes.prop_cycle'] += cycler(marker=markers)
    rcParams["text.usetex"] = True
    rcParams['text.latex.preamble'] = '\\usepackage{sfmath}'

    log_dir = os.path.join(os.path.dirname(__file__),
                           "../logs")

    dtype = [("NumElements", np.int64), ("NIteration", np.int64), ("ZipfSkewness", np.float64), ("UpdateRatio", np.float64),
             ("PurelyLocalCapacity", np.int64), ("UMAP_BUFSIZE",
                                                 np.int64), ("batch_blocking", np.int64),
             ("construction_duration", np.int64), ("query_duration", np.int64), ("query_read_cnt", np.int64), ("query_write_cnt", np.int64)]

    data = {}
    data["page"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_page_skiplist.log"), dtype=dtype, ndmin=2)
    data["local"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_local_skiplist.log"), dtype=dtype, ndmin=2)
    data["local+page"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_local+page_skiplist.log"), dtype=dtype, ndmin=2)
    data["hint"] = np.loadtxt(os.path.join(log_dir, "kvs_benchmark_with_hint_skiplist.log"), dtype=dtype, ndmin=2)

    for variant in data.keys():
        used_local_buf = (data[variant]["PurelyLocalCapacity"] + data[variant]["UMAP_BUFSIZE"]
                        * 4096) * 100 / (160 * data[variant]["NumElements"]) + 0.5
        query_swap_cnt = data[variant]["query_read_cnt"] + \
            data[variant]["query_write_cnt"]
        tmp = rfn.append_fields(data[variant], ("used_local_buf", "query_swap_cnt"), (
            used_local_buf, query_swap_cnt), dtypes=(np.int64, np.int64))
        tmp.sort(order="used_local_buf")
        data[variant] = tmp

    for zipf_skewness, update_ratio, subfigure_idx in ((0.8, 0.05, "a"), (0.8, 0.5, "b"), (1.3, 0.05, "c"), (1.3, 0.5, "d")):
        fig = plt.figure(figsize=(2.55, 1))
        ax = fig.add_subplot(1, 1, 1)
        ax.set_xlabel(r"$L =$(local memory usage)/(data size) [\%]")
        ax.set_ylabel(r"amount of swapped data")
        ax.yaxis.set_major_formatter(ticker.ScalarFormatter(useMathText=True))
        ax.ticklabel_format(style="sci", axis="y", scilimits=(0, 0))

        legendfig = plt.figure()
        lines = []
        labels = []

        ax.axvline(100, color="black", linestyle="dashed")

        for variant in data:
            datum = data[variant][(data[variant]["ZipfSkewness"] == zipf_skewness) & (
                data[variant]["UpdateRatio"] == update_ratio)]
            lines.append(ax.plot(
                datum["used_local_buf"],
                datum["query_swap_cnt"]
            ))
            labels.append(variant)

        x_max = np.max([data[variant]["used_local_buf"][-1] for variant in data])
        ax.set_xlim((0, x_max))
        ax.set_xticks(tuple(x_max * i // 8 for i in range(9)))

        (bottom, top) = ax.get_ylim()
        ax.set_ylim((0.0, top))

        fig.savefig(
            os.path.join(
                os.path.dirname(__file__), f"../charts/figure12{subfigure_idx}.pdf"
            ),
            bbox_inches="tight",
        )

        legendfig.legend(map(lambda x: x[0], lines), labels, ncol=4).get_frame().set_alpha(1.0)
        legendfig.savefig(
            os.path.join(
                os.path.dirname(__file__), f"../charts/figure12_legend.pdf"
            ),
            bbox_inches="tight",
        )


if __name__ == "__main__":
    main()
