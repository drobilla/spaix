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


DataSource = namedtuple("DataSource", "path label data")
PlotColumn = namedtuple("PlotColumn", "name label")
PlotScale = namedtuple("PlotScale", "show_error max_value divisor_col")


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


def plot(
    sources,
    out_filename,
    x_col,
    y_col,
    y_scale=PlotScale(False, None, None),
):
    """Plot results."""

    matplotlib.use("agg")
    import matplotlib.pyplot as plt  # pylint: disable=import-outside-toplevel

    dashes = get_dashes()
    markers = itertools.cycle(["o", "s", "v", "D", "*", "p", "P", "h", "X"])

    plt.clf()
    fig = plt.figure(figsize=(FIG_HEIGHT * math.sqrt(2), FIG_HEIGHT))
    ax = fig.add_subplot(111)  # pylint: disable=invalid-name

    ax.set_xlabel(x_col.label)
    ax.set_ylabel(y_col.label)

    min_col = y_col.name + "_min" if y_scale.show_error else y_col.name
    max_col = y_col.name + "_max" if y_scale.show_error else y_col.name

    ax.set_ylim([0.0, get_y_max(sources, y_col, y_scale)])
    ax.grid(linewidth=0.25, linestyle=":", color="0", dashes=[0.2, 1.6])
    ax.ticklabel_format(style="sci", scilimits=(4, 0), useMathText=True)
    ax.tick_params(axis="both", width=0.75)

    for source in sources:
        yerr = None
        if y_scale.show_error:
            yerr = [
                source.data[y_col.name] - source.data[min_col],
                source.data[max_col] - source.data[y_col.name],
            ]

        ax.errorbar(
            source.data[x_col.name],
            get_y_data(source, y_col, y_scale),
            yerr=yerr,
            label=source.label,
            marker=next(markers),
            linestyle=next(dashes),
            markersize=3.0,
            linewidth=1.0,
            elinewidth=0.75,
            capthick=0.75,
            capsize=4.0,
        )

    plt.legend()
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
        sources = []
        for insert in ["linear"]:
            for split in ["linear", "quadratic"]:
                for prog in args.program:
                    path = tsv_path(args, prog, insert, split)
                    if os.path.exists(path) and os.path.getsize(path) > 0:
                        label = f"{bench_name(prog)} {split}"
                        sources += [read_tsv(path, label)]

        def max_value(cols):
            """Return the maximum value in the given column in all sources."""
            result = 0.0
            for source in sources:
                for col in cols:
                    if args.no_error or col.endswith("_max"):
                        result = max(result, source.data[col].max())
                    else:
                        result = max(result, source.data[col + "_max"].max())

            return result

        plot(
            sources,
            os.path.join(args.dir, "insert.svg"),
            PlotColumn("n", "Size"),
            PlotColumn("t_ins", "Insert time (s)"),
            PlotScale(not args.no_error, max_value(["t_ins"]), None),
        )

        plot(
            sources,
            os.path.join(args.dir, "throughput.svg"),
            PlotColumn("n", "Size"),
            PlotColumn("n", "Insert throughput (/s)"),
            PlotScale(False, None, "elapsed"),
        )

        plot(
            sources,
            os.path.join(args.dir, "iter.svg"),
            PlotColumn("n", "Size"),
            PlotColumn("t_iter", "Range query time (s)"),
            PlotScale(not args.no_error, None, None),
        )

        plot(
            sources,
            os.path.join(args.dir, "q_dirs.svg"),
            PlotColumn("n", "Size"),
            PlotColumn("q_dirs", "Directory nodes searched"),
            PlotScale(not args.no_error, max_value(["q_dirs_max"]), None),
        )

        plot(
            sources,
            os.path.join(args.dir, "q_dats.svg"),
            PlotColumn("n", "Size"),
            PlotColumn("q_dats", "Leaf nodes searched"),
            PlotScale(not args.no_error, max_value(["q_dats_max"]), None),
        )

        html_path = os.path.join(args.dir, "benchmarks.html")
        with open(html_path, "w", encoding="utf-8") as html:
            html.write(
                """<!DOCTYPE html>
<html lang="en">
  <head><title>R-tree Benchmarks</title>
  <meta charset="utf-8"/></head>
  <body>
    <figure><img src="insert.svg" alt="Insert time"/></figure>
    <figure><img src="throughput.svg" alt="Insert throughput"/></figure>
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
