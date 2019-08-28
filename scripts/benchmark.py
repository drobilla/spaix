#!/usr/bin/env python

import optparse
import pandas
import subprocess
import sys
import matplotlib
import itertools
import math
import os


def get_dashes():
    "Generator for plot line dash patterns"
    dash = 2.0
    space = dot = 0.75

    yield []  # Solid
    yield [dash, space]  # Dashed
    yield [dot, space]  # Dotted

    # Dash-dots, with increasing number of dots for each line
    for i in itertools.count(2):
        yield [dash, space] + [dot, space] * (i - 1)


class DataSource:
    def __init__(self, path, label):
        self.path = path
        self.label = label
        self.data = pandas.read_csv(path, sep="\t")


def plot(sources, out_filename, x_col, x_label, y_col, y_label, y_max=None):
    "Plot results"

    matplotlib.use("agg")
    import matplotlib.pyplot as plt

    fig_height = 4.0
    dashes = get_dashes()
    markers = itertools.cycle(["o", "s", "v", "D", "*", "p", "P", "h", "X"])

    plt.clf()
    fig = plt.figure(figsize=(fig_height * math.sqrt(2), fig_height))
    ax = fig.add_subplot(111)

    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)

    if y_max is None:
        y_max = 0.0
        for source in sources:
            y_max = max(y_max, source.data[y_col + "_max"].max())

    ax.set_ylim([0.0, y_max * 1.01])
    ax.grid(linewidth=0.25, linestyle=":", color="0", dashes=[0.2, 1.6])
    ax.ticklabel_format(style="sci", scilimits=(4, 0), useMathText=True)
    ax.tick_params(axis="both", width=0.75)

    min_col = y_col + "_min"
    max_col = y_col + "_max"

    for source in sources:
        ax.errorbar(
            source.data[x_col],
            source.data[y_col],
            yerr=[source.data[y_col] - source.data[min_col],
                  source.data[max_col] - source.data[y_col]],
            label=source.label,
            marker=next(markers),
            dashes=next(dashes),
            markersize=3.0,
            linewidth=1.0,
            elinewidth=0.75,
            capthick=0.75,
            capsize=4.0,
        )

    plt.legend()
    plt.savefig(out_filename, bbox_inches="tight", pad_inches=0.025)
    plt.close()
    sys.stderr.write("Wrote {}\n".format(out_filename))


def run(script_opts, bench_opts, out_path):
    "Run one benchmark and return the path to the output file"
    with open(out_path, "w") as out:
        subprocess.check_call([script_opts.program] + bench_opts, stdout=out)

    sys.stderr.write("Wrote {}\n".format(out_path))


if __name__ == "__main__":
    opt = optparse.OptionParser(usage="%prog [OPTION]...",
                                description="Benchmark R-tree variants\n")

    opt.add_option("--dir", type="string", default="build",
                   help="path to output directory")
    opt.add_option("--no-run", action="store_true",
                   help="do not run benchmarks")
    opt.add_option("--no-plot", action="store_true",
                   help="do not plot benchmarks")
    opt.add_option("--page-size", type="int", default=128,
                   help="page size for directory nodes")
    opt.add_option("--program", type="string", default="build/bench_RTree",
                   help="path to benchmarking program",)
    opt.add_option("--queries", type="int", default=32,
                   help="number of queries per step")
    opt.add_option("--seed", type="int", default=5489,
                   help="random number generator seed")
    opt.add_option("--size", type="int", default=1000000,
                   help="maximum number of elements")
    opt.add_option("--span", type="float", default=1000000.0,
                   help="dimension span")
    opt.add_option("--steps", type="int", default=10,
                   help="number of benchmarking steps")

    (options, args) = opt.parse_args()
    if len(args):
        opt.print_usage()
        sys.exit(1)

    bench_opts = [
        "--page-size", str(options.page_size),
        "--queries", str(options.queries),
        "--seed", str(options.seed),
        "--size", str(options.size),
        "--span", str(options.span),
        "--steps", str(options.steps),
    ]

    def tsv_path(insert, split):
        return os.path.join(options.dir,
                            "insert_{}_split_{}.tsv".format(insert, split))

    if not options.no_run:
        for insert in ("linear", "quadratic"):
            for split in ("linear", "quadratic"):
                if insert == "quadratic" and split == "linear":
                    continue  # Works, but is really awful and makes no sense

                run(options,
                    bench_opts + ["--insert", insert, "--split", split],
                    tsv_path(insert, split))

    def mathify(name):
        return "$m$" if name == "linear" else "$m^2$"

    if not options.no_plot:
        sources = []
        for insert in ("linear", "quadratic"):
            for split in ("linear", "quadratic"):
                path = tsv_path(insert, split)
                if os.path.exists(path):
                    label = "insert {}, split {}".format(mathify(insert),
                                                         mathify(split))
                    sources += [DataSource(path, label)]

        def max_value(cols):
            """Return the maximum value in the given column in all sources"""
            result = 0.0
            for source in sources:
                for col in cols:
                    result = max(result, source.data[col + "_max"].max())

            return result

        plot(sources, os.path.join(options.dir, "insert.svg"),
             "n", "Size", "t_ins", "Insert time (s)")

        plot(sources, os.path.join(options.dir, "iter.svg"),
             "n", "Size", "t_iter", "Range query time (s)")

        nodes_y_max = max_value(["q_dirs", "q_dats"])
        plot(sources, os.path.join(options.dir, "q_dirs.svg"),
             "n", "Size", "q_dirs", "Directory nodes searched")

        plot(sources, os.path.join(options.dir, "q_dats.svg"),
             "n", "Size", "q_dats", "Leaf nodes searched")

        html_path = os.path.join(options.dir, "benchmarks.html")
        with open(html_path, "w") as html:
            html.write("""<!DOCTYPE html>
<html lang="en">
  <head><title>R-tree Benchmarks</title>
  <meta charset="utf-8"/></head>
  <body>
    <figure><img src="insert.svg" alt="Insert time"/></figure>
    <figure><img src="iter.svg" alt="Range query time"/></figure>
    <figure><img src="q_dirs.svg" alt="Directory nodes searched"/></figure>
    <figure><img src="q_dats.svg" alt="Leaf nodes searched"/></figure>
  </body>
</html>
""")
            sys.stderr.write("Wrote {}\n".format(html_path))
