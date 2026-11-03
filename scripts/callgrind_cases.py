"""Tabulates per-case callgrind dumps written by `investm_bench --quick`.

Under callgrind the benchmark driver dumps one profile per case (labelled
with the case name), so each dump holds that case alone, without the setup
that builds its inputs. scripts/profile.sh runs it as the `cases` tool.

Usage:
  python3 scripts/callgrind_cases.py DIR [BASELINE_DIR] [--csv=PATH]

DIR holds the callgrind.out.N dumps of one run. With BASELINE_DIR the table
compares the two runs case by case (baseline first).
"""

import glob
import os
import sys

_TRIGGER = "desc: Trigger: Client Request: "


def load(directory):
    """Returns {case: {event: count}} for the dumps in `directory`."""
    cases = {}
    for path in glob.glob(os.path.join(directory, "callgrind.out*")):
        name, events, summary = None, None, None
        with open(path, encoding="utf-8", errors="replace") as f:
            for line in f:
                if line.startswith(_TRIGGER):
                    name = line[len(_TRIGGER):].strip()
                elif line.startswith("events:"):
                    events = line.split()[1:]
                elif line.startswith("summary:"):
                    summary = [int(x) for x in line.split()[1:]]
        # "before X" dumps hold the driver's own work between cases.
        if name and events and summary and not name.startswith("before "):
            cases[name] = dict(zip(events, summary))
    return cases


def _d1(c):
    return c.get("D1mr", 0) + c.get("D1mw", 0)


def main(argv):
    csv_path = None
    args = []
    for a in argv[1:]:
        if a.startswith("--csv="):
            csv_path = a[len("--csv="):]
        else:
            args.append(a)
    if not args or len(args) > 2:
        sys.exit(__doc__)
    after = load(args[0])
    before = load(args[1]) if len(args) == 2 else None
    rows = []
    if before is None:
        print(f"{'case':38s} {'Ir':>14s} {'D1 misses':>11s} {'LL misses':>10s}"
              f" {'br mispred':>11s}")
        for name, c in after.items():
            rows.append((name, c["Ir"], _d1(c),
                         c.get("DLmr", 0) + c.get("DLmw", 0), c.get("Bcm", 0)))
        for r in sorted(rows, key=lambda r: -r[1]):
            print(f"{r[0]:38s} {r[1]:14,d} {r[2]:11,d} {r[3]:10,d} "
                  f"{r[4]:11,d}")
        header = "case,ir,d1_misses,ll_misses,branch_mispredicts"
    else:
        print(f"{'case':38s} {'Ir before':>14s} {'Ir after':>14s} {'ratio':>8s}"
              f" {'D1 before':>10s} {'D1 after':>10s}")
        for name, a in after.items():
            b = before.get(name)
            if b is None:
                continue
            rows.append((name, b["Ir"], a["Ir"], b["Ir"] / max(a["Ir"], 1),
                         _d1(b), _d1(a)))
        for r in sorted(rows, key=lambda r: -r[3]):
            print(f"{r[0]:38s} {r[1]:14,d} {r[2]:14,d} {r[3]:7.2f}x "
                  f"{r[4]:10,d} {r[5]:10,d}")
        header = "case,ir_before,ir_after,ratio,d1_before,d1_after"
    if csv_path:
        with open(csv_path, "w", encoding="utf-8") as f:
            f.write(header + "\n")
            for r in rows:
                f.write(",".join(str(x) for x in r) + "\n")


if __name__ == "__main__":
    main(sys.argv)
