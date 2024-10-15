#!/usr/bin/env python

# Copyright 2020-2024 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

"""Benchmark R-tree variants."""

import itertools
import math
import argparse
import os
import subprocess
import sys

from collections import namedtuple
from enum import Enum

import matplotlib
import pandas


FIG_HEIGHT = 4.0


def get_dashes():
    """Generator for plot line dash patterns."""

    dash = 2
    space = dot = 1

    yield (0, ())  # Solid
    yield (0, (dash, space))  # Dashed
    yield (0, (dot, space))  # Dotted

    # Dash-dots, with increasing number of dots for each line
    for i in itertools.count(2):
        yield (0, (dash, space) + (dot, space) * (i - 1))


AxisScale = Enum("AxisScale", ["LINEAR", "LOG"])
ErrorStyle = Enum("ErrorStyle", ["NONE", "BARS", "REGION"])

DataSource = namedtuple("DataSource", "path label data")
PlotColumn = namedtuple("PlotColumn", "name label")
PlotScale = namedtuple(
    "PlotScale", "axis_scale error_style max_value divisor_col"
)

PlotState = namedtuple("PlotState", "ax y_scale dashes markers")


def read_tsv(path, label):
    """Read a TSV file with pandas and return a DataSource."""
    return DataSource(path, label, pandas.read_csv(path, sep="\t"))


def get_y_data(source, y_col, y_scale):
    """Return the data for a source scaled if necessary."""

    y_data = source.data[y_col.name]

    if y_scale.divisor_col is not None:
        y_data = y_data / source.data[y_scale.divisor_col]

    return y_data


def get_y_max(sources, y_col, y_scale):
    """Return the maximum y value in all plot sources for axis scaling."""

    y_max = y_scale.max_value
    if y_max is None:
        y_max = 0.0
        for source in sources:
            y_max = max(y_max, get_y_data(source, y_col, y_scale).max())

    return y_max * 1.01


def plot_error_bars(state, source, x_col, y_col):
    """Plot a single source's results as a line with optional error bars."""

    x = source.data[x_col.name]
    y = get_y_data(source, y_col, state.y_scale)

    yerr = None
    if state.y_scale.error_style == ErrorStyle.BARS:
        yerr = [
            source.data[y_col.name] - source.data[f"{y_col.name}_min"],
            source.data[f"{y_col.name}_max"] - source.data[y_col.name],
        ]

    line = state.ax.errorbar(
        x,
        y,
        capsize=4.0,
        capthick=0.75,
        elinewidth=0.75,
        label=source.label,
        linestyle=next(state.dashes),
        linewidth=1.0,
        marker=next(state.markers),
        markersize=3.0,
        yerr=yerr,
    )

    return (line[0],)


def plot_region(state, source, x_col, y_col):
    """Plot a single source's results as a line within an error region."""

    x = source.data[x_col.name]
    y = get_y_data(source, y_col, state.y_scale)

    min_col = y_col.name + "_min"
    max_col = y_col.name + "_max"

    marker = next(state.markers)
    linestyle = next(state.dashes)

    line = state.ax.plot(
        x,
        y,
        alpha=1.0,
        label=source.label,
        linestyle=linestyle,
        linewidth=1.0,
        marker=marker,
        markersize=3.0,
    )

    color = line[0].get_color()
    region = state.ax.fill_between(
        x,
        source.data[min_col],
        source.data[max_col],
        alpha=0.25,
        color=color,
        linestyle=linestyle,
        linewidth=0.5,
    )

    for bound_col in [min_col, max_col]:
        state.ax.plot(
            x,
            source.data[bound_col],
            alpha=0.5,
            color=color,
            linestyle=linestyle,
            linewidth=0.25,
            marker=marker,
            markersize=0.5,
        )

    return (line[0], region)


def plot(
    sources,
    out_filename,
    x_col,
    y_col,
    y_scale=PlotScale(AxisScale.LINEAR, ErrorStyle.NONE, None, None),
):
    """Plot results."""

    matplotlib.use("agg")
    import matplotlib.pyplot as plt  # pylint: disable=import-outside-toplevel

    plt.clf()
    fig = plt.figure(figsize=(FIG_HEIGHT * math.sqrt(2), FIG_HEIGHT))
    ax = fig.add_subplot(111)  # pylint: disable=invalid-name

    ax.set_xlabel(x_col.label)
    ax.set_ylabel(y_col.label)

    ax.grid(linewidth=0.25, linestyle=":", color="0", dashes=[0.2, 1.6])
    ax.ticklabel_format(style="sci", scilimits=(4, 0), useMathText=True)
    ax.tick_params(axis="both", width=0.75)

    if y_scale.axis_scale == AxisScale.LOG:
        plt.yscale("log")

    y_max = get_y_max(sources, y_col, y_scale)
    if y_max > 1000:
        y_min = 1.0 if y_scale.axis_scale == AxisScale.LOG else 0.0
        ax.set_ylim([y_min, get_y_max(sources, y_col, y_scale)])

    state = PlotState(
        ax,
        y_scale,
        get_dashes(),
        itertools.cycle(["o", "s", "v", "D", "*", "p", "P", "h", "X"]),
    )

    legend_entries = []
    for source in sources:
        if y_scale.error_style == ErrorStyle.REGION:
            legend_entries += [plot_region(state, source, x_col, y_col)]
        else:
            legend_entries += [plot_error_bars(state, source, x_col, y_col)]

    ax.legend(
        legend_entries,
        [source.label for source in sources],
        borderaxespad=0.25,
        borderpad=0.25,
        columnspacing=0,
        fancybox=False,
        framealpha=0.75,
        handletextpad=0.25,
        labelspacing=0.25,
    )

    plt.savefig(out_filename, bbox_inches="tight", pad_inches=0.025)
    plt.close()
    sys.stderr.write(f"Wrote {out_filename}\n")


def bench_name(prog):
    """Return a reasonable base name for a benchmark row."""

    name = os.path.basename(prog)
    if name.startswith("bench_"):
        name = name[6:]
    if name.endswith("_rtree"):
        name = name[0:-6]

    return name


def tsv_path(options, prog, insert, split):
    """Return the path of the TSV file for a given benchmark run."""

    return os.path.join(
        options.dir,
        f"{bench_name(prog)}_insert_{insert}_split_{split}.tsv".format(),
    )


def run(script_opts, bench_opts, insert, split):
    """Run one benchmark and return the path to the output file."""

    for prog in script_opts.program:
        out_path = tsv_path(script_opts, prog, insert, split)
        with open(out_path, "w", encoding="utf-8") as out:
            alg_opts = ["--insert", insert, "--split", split]
            subprocess.check_call([prog] + bench_opts + alg_opts, stdout=out)

        sys.stderr.write(f"Wrote {out_path}\n")


def plot_all(sources, error_style, out_dir):
    """Plot all results."""

    def max_value(cols):
        """Return the maximum value in the given column in all sources."""
        result = 0.0
        for source in sources:
            for col in cols:
                result = max(result, source.data[col].max())

        return result

    plot(
        sources,
        os.path.join(out_dir, "insert.svg"),
        PlotColumn("n", "Size"),
        PlotColumn("t_ins", "Insert time (s)"),
        PlotScale(AxisScale.LOG, error_style, max_value(["t_ins_max"]), None),
    )

    plot(
        sources,
        os.path.join(out_dir, "throughput.svg"),
        PlotColumn("n", "Size"),
        PlotColumn("n", "Insert throughput (/s)"),
        PlotScale(AxisScale.LINEAR, ErrorStyle.NONE, None, "elapsed"),
    )

    plot(
        sources,
        os.path.join(out_dir, "iter.svg"),
        PlotColumn("n", "Size"),
        PlotColumn("t_iter", "Range query time (s)"),
        PlotScale(
            AxisScale.LINEAR, error_style, max_value(["t_iter_max"]), None
        ),
    )

    spaix_sources = list(filter(lambda s: "spaix" in s.path, sources))
    nodes_max = max_value(["q_dirs_max", "q_dats_max"])

    plot(
        spaix_sources,
        os.path.join(out_dir, "q_dirs.svg"),
        PlotColumn("n", "Size"),
        PlotColumn("q_dirs", "Directory nodes searched"),
        PlotScale(AxisScale.LOG, error_style, nodes_max, None),
    )

    plot(
        spaix_sources,
        os.path.join(out_dir, "q_dats.svg"),
        PlotColumn("n", "Size"),
        PlotColumn("q_dats", "Leaf nodes searched"),
        PlotScale(AxisScale.LOG, error_style, nodes_max, None),
    )


def main(argv):
    """Run the command-line utility."""

    parser = argparse.ArgumentParser(
        usage="%(prog)s [OPTION]...",
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    parser.add_argument("--dir", default=".", help="path to output directory")
    parser.add_argument(
        "--no-error", action="store_true", help="do not show error bars"
    )
    parser.add_argument(
        "--no-run", action="store_true", help="do not run benchmarks"
    )
    parser.add_argument(
        "--no-plot", action="store_true", help="do not plot benchmarks"
    )
    parser.add_argument(
        "--page-size",
        type=int,
        default=512,
        metavar="BYTES",
        help="page size for directory nodes",
    )
    parser.add_argument(
        "--inline", action="store_true", help="inline data nodes in parents"
    )
    parser.add_argument(
        "--program",
        default=["test/benchmark/bench_spaix_rtree"],
        action="append",
        help="path to benchmarking program",
    )
    parser.add_argument(
        "--queries", type=int, default=32, help="number of queries per step"
    )
    parser.add_argument(
        "--seed", type=int, default=5489, help="random number generator seed"
    )
    parser.add_argument(
        "--size",
        type=int,
        default=1000000,
        help="maximum number of elements",
    )
    parser.add_argument(
        "--span", type=float, default=10000000.0, help="dimension span"
    )
    parser.add_argument(
        "--steps", type=int, default=10, help="number of benchmarking steps"
    )

    args = parser.parse_args(argv[1:])
    if len(args.program) > 1:
        args.program = args.program[1:]  # Remove default

    bench_opts = [
        "--page-size",
        str(args.page_size),
        "--placement",
        "inline" if args.inline else "separate",
        "--queries",
        str(args.queries),
        "--seed",
        str(args.seed),
        "--size",
        str(args.size),
        "--span",
        str(args.span),
        "--steps",
        str(args.steps),
    ]

    if not args.no_run:
        for insert in ["linear"]:
            for split in ["linear", "quadratic"]:
                run(args, bench_opts, insert, split)

    if not args.no_plot:
        error_style = ErrorStyle.NONE if args.no_error else ErrorStyle.REGION

        sources = []
        for insert in ["linear"]:
            for split in ["linear", "quadratic"]:
                for prog in args.program:
                    path = tsv_path(args, prog, insert, split)
                    if os.path.exists(path) and os.path.getsize(path) > 0:
                        label = f"{bench_name(prog)} {split}"
                        sources += [read_tsv(path, label)]

        plot_all(sources, error_style, args.dir)

        html_path = os.path.join(args.dir, "benchmarks.html")
        with open(html_path, "w", encoding="utf-8") as html:
            html.write(
                """<!DOCTYPE html>
<html lang="en">
  <head><title>R-tree Benchmarks</title>
  <meta charset="utf-8"/></head>
  <body>
    <figure><img src="throughput.svg" alt="Insert throughput"/></figure>
    <figure><img src="insert.svg" alt="Insert time"/></figure>
    <figure><img src="iter.svg" alt="Range query time"/></figure>
    <figure><img src="q_dirs.svg" alt="Directory nodes searched"/></figure>
    <figure><img src="q_dats.svg" alt="Leaf nodes searched"/></figure>
  </body>
</html>
"""
            )
            sys.stderr.write(f"Wrote {html_path}\n")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
