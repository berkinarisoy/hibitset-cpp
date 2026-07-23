#!/usr/bin/env python3
"""Update the benchmark tables in README.md from Google Benchmark JSON output.

Only the numeric rows of each table are replaced; headings, prose, and table
order are left exactly as they are in the README.
"""
import argparse
import json
import re
from collections import defaultdict
from pathlib import Path

NAME_RE = re.compile(r"^BM_(\w+)_(hibitset|stdset|dynset)<1 << (\d+)>/(\d+)$")
MEMORY_RE = re.compile(r"^BM_memory<1 << (\d+)>/(\d+)$")

IMPLS = ["hibitset", "stdset", "dynset"]
# iterate reports SetItemsProcessed for a whole-container batch; use the
# per-element rate so it reads as "cost per op", like the other tables.
# construct stays a batch total: "how long to build this container" is the
# more useful number there than an amortized per-element rate.
PER_ELEMENT_OPS = {"iterate"}

# (README heading text, op key in NAME_RE) in the exact order they appear.
OP_SECTIONS = [
    ("### Find next set bit time per operation (ns)", "next"),
    ("### Find previous set bit time per operation (ns)", "prev"),
    ("### Iterate over set bits time per element (ns)", "iterate"),
    ("### AND time per operation (ns)", "and"),
    ("### OR time per operation (ns)", "or"),
    ("### XOR time per operation (ns)", "xor"),
    ("### Total initialization time (ns)", "construct"),
]
MEMORY_HEADING = "### Memory usage"

MEDALS = ["🥇", "🥈", "🥉"]


def fmt_ns(x):
    if x < 10:
        return f"{x:.2f}"
    if x < 1000:
        return f"{x:.1f}"
    return f"{x:,.0f}"


def fmt_bytes(x):
    return f"{x:,.0f}"


def rank_cells(values, fmt):
    """values: dict impl -> number or None. Returns dict impl -> formatted str, medal-ranked low-to-high."""
    order = sorted((k for k, v in values.items() if v is not None), key=values.get)
    rank = {k: i for i, k in enumerate(order)}
    return {
        k: f"{MEDALS[rank[k]]} {fmt(v)}" if v is not None else "-"
        for k, v in values.items()
    }


def cap_label(shift):
    return f"2^{shift} ({2 ** int(shift):,})"


def density_label(per_mille):
    return f"{int(per_mille) / 10:g}%"


def collect_op_data(benchmarks):
    # op -> (capacity, density) -> impl -> cpu_time_ns
    data = defaultdict(lambda: defaultdict(dict))
    for b in benchmarks:
        m = NAME_RE.match(b["name"])
        if not m:
            continue
        op, impl, cap, density = m.groups()
        value = 1e9 / b["items_per_second"] if op in PER_ELEMENT_OPS else b["cpu_time"]
        data[op][(int(cap), int(density))][impl] = value
    return data


def collect_memory_data(benchmarks):
    rows = []
    for b in benchmarks:
        m = MEMORY_RE.match(b["name"])
        if not m:
            continue
        cap, density = m.groups()
        rows.append((int(cap), int(density), b))
    return sorted(rows)


def op_rows(times_by_key):
    lines = []
    for (cap, density), times in sorted(times_by_key.items()):
        cells = rank_cells({impl: times.get(impl) for impl in IMPLS}, fmt_ns)
        lines.append(
            f"| {cap_label(cap)} | {density_label(density)} | "
            f"{cells['hibitset']} | {cells['stdset']} | {cells['dynset']} |"
        )
    return lines


def memory_rows(rows):
    lines = []
    for cap, density, b in rows:
        cells = rank_cells({impl: b[f"bytes_{impl}"] for impl in IMPLS}, fmt_bytes)
        lines.append(
            f"| {cap_label(cap)} | {density_label(density)} | "
            f"{cells['hibitset']} | {cells['stdset']} | {cells['dynset']} |"
        )
    return lines


def replace_table(lines, heading, new_rows):
    """Swap the data rows of the table under `heading` for `new_rows`, leaving
    the heading, any prose, and the table header/separator untouched."""
    start = next((i for i, l in enumerate(lines) if l.strip() == heading), None)
    if start is None:
        raise SystemExit(f"README heading not found: {heading!r}")

    header = start + 1
    while header < len(lines) and not lines[header].startswith("| Universe size"):
        header += 1
    if header >= len(lines):
        raise SystemExit(f"table header not found under heading: {heading!r}")

    data_start = header + 2  # past header row + separator row
    data_end = data_start
    while data_end < len(lines) and lines[data_end].startswith("|"):
        data_end += 1

    return lines[:data_start] + new_rows + lines[data_end:]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", default="build-bench/bench_results.json", type=Path)
    parser.add_argument("-o", "--output", default="README.md", type=Path)
    parser.add_argument("--readme", default="README.md", type=Path, help="README to read the existing table layout from")
    args = parser.parse_args()

    result = json.loads(args.input.read_text())
    benchmarks = result["benchmarks"]
    op_data = collect_op_data(benchmarks)
    mem_data = collect_memory_data(benchmarks)

    lines = args.readme.read_text().splitlines()
    for heading, op in OP_SECTIONS:
        lines = replace_table(lines, heading, op_rows(op_data[op]))
    lines = replace_table(lines, MEMORY_HEADING, memory_rows(mem_data))

    args.output.write_text("\n".join(lines) + "\n")
    print(f"updated {args.output}")


if __name__ == "__main__":
    main()
