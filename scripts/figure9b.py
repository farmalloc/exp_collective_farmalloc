#!/usr/bin/env python3
import os.path
import colorsys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as m_col
from matplotlib import rcParams, ticker
import numpy.lib.recfunctions as rfn
from cycler import cycler


def main():
    rcParams["font.family"] = "sans-serif"
    rcParams["font.sans-serif"] = ["IPAexGothic"]
    rcParams["pdf.fonttype"] = 42  # TrueType
    rcParams["font.size"] = 8  # footnotesize=8
    rcParams['lines.markersize'] = 3
    markers = ["o", "x", "v", "D", "s", "*", "^", "d"]
    plt.style.use('tableau-colorblind10')
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    plt.rcParams['axes.prop_cycle'] += cycler(marker=markers)

    log_dir = os.path.join(os.path.dirname(__file__),
                           "../logs")

    dtype = [("NumElements", np.int64), ("PurelyLocalCapacity", np.int64), ("batch_blocking", np.int64),
             ("construction_duration", np.int64), ("purely-local", np.int64), ("in-page", np.int64), ("cross-page", np.int64)]
    edge_types = ("purely-local", "in-page", "cross-page")

    plot_data = {}
    plot_data["page"] = np.loadtxt(os.path.join(log_dir, "analyze_edges_of_page_skiplist.log"), dtype=dtype, ndmin=2)
    plot_data["local"] = np.loadtxt(os.path.join(log_dir, "analyze_edges_of_local_skiplist.log"), dtype=dtype, ndmin=2)
    plot_data["local+page"] = np.loadtxt(os.path.join(log_dir, "analyze_edges_of_local+page_skiplist.log"), dtype=dtype, ndmin=2)
    plot_data["hint"] = np.loadtxt(os.path.join(log_dir, "analyze_edges_of_hint_skiplist.log"), dtype=dtype, ndmin=2)

    values = np.concatenate(tuple(plot_data.values()))
    keys = plot_data.keys()

    num_edges = np.sum([values[e] for e in edge_types], axis=0)
    values = rfn.append_fields(values, [e + "_ratio" for e in edge_types], [
                               values[e] / num_edges for e in edge_types], dtypes=[np.float64 for e in edge_types])

    fig = plt.figure(figsize=(5.9, 0.84))
    ax = fig.add_subplot(1, 1, 1)
    ax.set_xlabel(r"composition ratio of links in each category [%]")

    positions = list(range(len(keys)))
    offsets = np.zeros(len(keys), dtype=np.float64)
    for i, e in enumerate(edge_types):
        persentage = values[e + "_ratio"] * 100
        stock = ax.barh(positions, persentage, left=offsets,
                        label=e, tick_label=list(keys))

        bar_color = colors[i]
        brightness = colorsys.rgb_to_hls(*m_col.to_rgb(bar_color))[1]
        ax.bar_label(stock, labels=[("" if r < 0.1 else str(int(r)) + "%") for r in persentage], label_type='center',
                     color=("white" if brightness < 0.5 else "black"))
        offsets += persentage

    ax.set_xlim((0, 100))
    ax.invert_yaxis()

    plt.legend(bbox_to_anchor=(1, 1), loc='lower right',
               borderaxespad=1, ncol=3).get_frame().set_alpha(1.0)

    plt.savefig(
        os.path.join(
            os.path.dirname(
                __file__), f"../charts/figure9b.pdf"
        ),
        bbox_inches="tight",
    )


if __name__ == "__main__":
    main()
