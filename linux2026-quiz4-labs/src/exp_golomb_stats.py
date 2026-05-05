#!/usr/bin/env python3
from __future__ import annotations

import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "results" / "exp_golomb" / "exp_golomb_entropy.csv"
PLOT = ROOT / "results" / "exp_golomb" / "exp_golomb_overhead.png"


def exp_golomb_len(v: int) -> int:
    return 2 * int(math.floor(math.log2(v + 1))) + 1


def find_vmax(p: float, threshold: float = 1e-12) -> tuple[int, float]:
    v = 0
    tail = 1.0
    q = 1.0 - p
    while tail >= threshold:
        v += 1
        tail = q ** (v + 1)
        if v > 10_000_000:
            raise RuntimeError("v_max search failed")
    return v, tail


def main() -> int:
    OUT.parent.mkdir(parents=True, exist_ok=True)
    rows = []
    for i in range(1, 20):
        p = i / 20.0
        vmax, tail = find_vmax(p)
        q = 1.0 - p
        entropy = 0.0
        exp_len = 0.0
        for v in range(vmax + 1):
            prob = p * (q ** v)
            entropy -= prob * math.log2(prob)
            exp_len += prob * exp_golomb_len(v)
        rows.append((p, entropy, exp_len, exp_len - entropy, tail, vmax))

    with OUT.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["p", "entropy_bits", "expected_exp_golomb_len", "overhead_bits", "tail_probability", "v_max"])
        for row in rows:
            writer.writerow([f"{row[0]:.2f}", f"{row[1]:.8f}", f"{row[2]:.8f}", f"{row[3]:.8f}", f"{row[4]:.12e}", row[5]])

    try:
        import matplotlib.pyplot as plt  # type: ignore
    except Exception:
        print("SKIPPED: missing matplotlib")
        return 0

    ps = [row[0] for row in rows]
    overhead = [row[3] for row in rows]
    plt.figure(figsize=(8, 4))
    plt.plot(ps, overhead, marker="o")
    plt.xlabel("p")
    plt.ylabel("overhead bits")
    plt.title("Exp-Golomb expected length overhead")
    plt.tight_layout()
    plt.savefig(PLOT)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
