#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import math
import struct
import zlib
from pathlib import Path


BUCKETS = 1024
KEY_START = 0
KEY_END = 10000
KEY_COUNT = KEY_END - KEY_START + 1
INPUT_BITS = 32
OUTPUT_BITS = 32
SAMPLES = 2048

HASH_CONSTANTS = [
    0x61C88647,
    0x9E3779B9,
    0x80000000,
    0x12345678,
]

HASH_LABELS = [f"0x{value:08X}" for value in HASH_CONSTANTS]


def u32(value: int) -> int:
    return value & 0xFFFFFFFF


def rot32(value: int, shift: int) -> int:
    value = u32(value)
    shift &= 31
    return u32((value << shift) | (value >> (32 - shift)))


def multiplicative_bucket(key: int, constant: int) -> int:
    return ((u32(key) * u32(constant)) & 0xFFFFFFFF) >> 22


def jenkins_final(a: int, b: int, c: int) -> tuple[int, int, int]:
    a = u32(a)
    b = u32(b)
    c = u32(c)

    c ^= b
    c = u32(c - rot32(b, 14))
    a ^= c
    a = u32(a - rot32(c, 11))
    b ^= a
    b = u32(b - rot32(a, 25))
    c ^= b
    c = u32(c - rot32(b, 16))
    a ^= c
    a = u32(a - rot32(c, 4))
    b ^= a
    b = u32(b - rot32(a, 14))
    c ^= b
    c = u32(c - rot32(b, 24))
    return a, b, c


def compute_occupancy(constant: int) -> list[int]:
    counts = [0] * BUCKETS
    for key in range(KEY_START, KEY_END + 1):
        counts[multiplicative_bucket(key, constant)] += 1
    return counts


def occupancy_stats(counts: list[int]) -> dict[str, float | int]:
    expected = KEY_COUNT / BUCKETS
    collisions = sum(max(0, count - 1) for count in counts)
    distinct = sum(1 for count in counts if count)
    chi_square = sum(((count - expected) ** 2) / expected for count in counts)
    return {
        "keys": KEY_COUNT,
        "distinct_buckets": distinct,
        "collisions": collisions,
        "collision_rate": collisions / KEY_COUNT,
        "expected_per_bucket": expected,
        "chi_square": chi_square,
        "min_bucket": min(counts),
        "max_bucket": max(counts),
    }


def ensure_canvas(width: int, height: int, color: tuple[int, int, int, int]) -> list[bytearray]:
    pixel = bytes(color)
    return [bytearray(pixel * width) for _ in range(height)]


def set_pixel(canvas: list[bytearray], x: int, y: int, color: tuple[int, int, int, int]) -> None:
    if x < 0 or y < 0 or y >= len(canvas) or x * 4 + 3 >= len(canvas[0]):
        return
    row = canvas[y]
    offset = x * 4
    row[offset : offset + 4] = bytes(color)


def fill_rect(
    canvas: list[bytearray],
    x0: int,
    y0: int,
    x1: int,
    y1: int,
    color: tuple[int, int, int, int],
) -> None:
    height = len(canvas)
    width = len(canvas[0]) // 4
    x0 = max(0, min(width, x0))
    y0 = max(0, min(height, y0))
    x1 = max(0, min(width, x1))
    y1 = max(0, min(height, y1))
    if x0 >= x1 or y0 >= y1:
        return
    row_fill = bytes(color) * (x1 - x0)
    for y in range(y0, y1):
        row = canvas[y]
        row[x0 * 4 : x1 * 4] = row_fill


def hline(canvas: list[bytearray], y: int, color: tuple[int, int, int, int]) -> None:
    if 0 <= y < len(canvas):
        canvas[y][:] = bytes(color) * (len(canvas[0]) // 4)


def lerp_channel(a: int, b: int, t: float) -> int:
    return round(a + (b - a) * t)


def lerp_color(
    a: tuple[int, int, int, int],
    b: tuple[int, int, int, int],
    t: float,
) -> tuple[int, int, int, int]:
    return tuple(lerp_channel(x, y, t) for x, y in zip(a, b))  # type: ignore[return-value]


def diverging_color(value: float) -> tuple[int, int, int, int]:
    value = max(0.0, min(1.0, value))
    low = (33, 102, 172, 255)
    mid = (247, 247, 247, 255)
    high = (178, 24, 43, 255)
    if value <= 0.5:
        return lerp_color(low, mid, value * 2.0)
    return lerp_color(mid, high, (value - 0.5) * 2.0)


def gradient_color(value: float) -> tuple[int, int, int, int]:
    value = max(0.0, min(1.0, value))
    stops = [
        (0.0, (49, 54, 149, 255)),
        (0.25, (69, 117, 180, 255)),
        (0.5, (255, 255, 255, 255)),
        (0.75, (252, 141, 89, 255)),
        (1.0, (165, 0, 38, 255)),
    ]
    for (start_t, start_color), (end_t, end_color) in zip(stops, stops[1:]):
        if value <= end_t:
            span = end_t - start_t
            local_t = 0.0 if span == 0 else (value - start_t) / span
            return lerp_color(start_color, end_color, local_t)
    return stops[-1][1]


def write_png(path: Path, width: int, height: int, canvas: list[bytearray]) -> None:
    raw = bytearray()
    for row in canvas:
        raw.append(0)
        raw.extend(row)

    def chunk(tag: bytes, data: bytes) -> bytes:
        return (
            struct.pack("!I", len(data))
            + tag
            + data
            + struct.pack("!I", zlib.crc32(tag + data) & 0xFFFFFFFF)
        )

    header = struct.pack("!IIBBBBB", width, height, 8, 6, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) + chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b"")
    path.write_bytes(png)


def render_histogram(counts: list[int], out_path: Path) -> None:
    width = BUCKETS * 2
    height = 280
    top_margin = 14
    bottom_margin = 20
    plot_height = height - top_margin - bottom_margin
    expected = KEY_COUNT / BUCKETS
    max_count = max(counts)
    scale_max = max(max_count, expected * 1.25)
    canvas = ensure_canvas(width, height, (248, 248, 248, 255))

    # Baseline and expected occupancy marker.
    hline(canvas, height - bottom_margin, (210, 210, 210, 255))
    expected_y = height - bottom_margin - 1 - round((expected / scale_max) * (plot_height - 1))
    hline(canvas, expected_y, (220, 88, 42, 255))

    for bucket, count in enumerate(counts):
        bar_height = round((count / scale_max) * (plot_height - 1))
        x0 = bucket * 2
        x1 = x0 + 2
        y1 = height - bottom_margin
        y0 = y1 - bar_height
        fill_rect(canvas, x0, y0, x1, y1, (37, 99, 177, 255))
    write_png(out_path, width, height, canvas)


def render_heatmap(matrix: list[list[float]], out_path: Path) -> None:
    cell = 16
    size = 32 * cell
    canvas = ensure_canvas(size, size, (245, 245, 245, 255))
    for row in range(32):
        for col in range(32):
            color = diverging_color(matrix[row][col])
            x0 = col * cell
            y0 = row * cell
            fill_rect(canvas, x0, y0, x0 + cell, y0 + cell, color)
    for step in range(0, size, cell):
        hline(canvas, step, (232, 232, 232, 255))
        for y in range(size):
            set_pixel(canvas, step, y, (232, 232, 232, 255))
    write_png(out_path, size, size, canvas)


def write_bucket_csv(out_path: Path, per_constant_counts: list[list[int]]) -> None:
    with out_path.open("w", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(["bucket", *HASH_LABELS])
        for bucket in range(BUCKETS):
            writer.writerow([bucket, *[counts[bucket] for counts in per_constant_counts]])


def write_stats_csv(out_path: Path, per_constant_counts: list[list[int]], stats: list[dict[str, float | int]]) -> None:
    with out_path.open("w", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(
            [
                "constant",
                "keys",
                "distinct_buckets",
                "collisions",
                "collision_rate",
                "expected_per_bucket",
                "chi_square",
                "min_bucket",
                "max_bucket",
            ]
        )
        for label, stat in zip(HASH_LABELS, stats):
            writer.writerow(
                [
                    label,
                    stat["keys"],
                    stat["distinct_buckets"],
                    stat["collisions"],
                    f"{stat['collision_rate']:.6f}",
                    f"{stat['expected_per_bucket']:.6f}",
                    f"{stat['chi_square']:.6f}",
                    stat["min_bucket"],
                    stat["max_bucket"],
                ]
            )


def write_hash_summary(out_path: Path, stats: list[dict[str, float | int]]) -> None:
    lines = [
        "Hash occupancy study",
        f"keys: {KEY_START}..{KEY_END} inclusive ({KEY_COUNT} total)",
        f"buckets: {BUCKETS}",
        "bucket formula: ((key * constant) & 0xffffffff) >> 22",
        f"expected occupancy per bucket: {KEY_COUNT / BUCKETS:.6f}",
        "",
        "Constant        Distinct  Collisions  Collision rate  Chi-square  Min  Max",
        "--------------  --------  ----------  --------------  ----------  ---  ---",
    ]
    for label, stat in zip(HASH_LABELS, stats):
        lines.append(
            f"{label}  {stat['distinct_buckets']:8d}  {stat['collisions']:10d}  "
            f"{stat['collision_rate']:.6f}      {stat['chi_square']:.3f}  "
            f"{stat['min_bucket']:3d}  {stat['max_bucket']:3d}"
        )
    out_path.write_text("\n".join(lines) + "\n")


def compute_avalanche(sample_count: int) -> list[list[float]]:
    fixed_b = 0x9E3779B9
    fixed_c = 0x243F6A88
    flip_counts = [[0 for _ in range(OUTPUT_BITS)] for _ in range(INPUT_BITS)]
    for sample_index in range(sample_count):
        base_a = u32(sample_index * 0x61C88647)
        _, _, base_out = jenkins_final(base_a, fixed_b, fixed_c)
        for input_bit in range(INPUT_BITS):
            flipped_a = base_a ^ (1 << input_bit)
            _, _, flipped_out = jenkins_final(flipped_a, fixed_b, fixed_c)
            diff = base_out ^ flipped_out
            for output_bit in range(OUTPUT_BITS):
                flip_counts[input_bit][output_bit] += (diff >> output_bit) & 1
    return [[count / sample_count for count in row] for row in flip_counts]


def write_avalanche_csv(out_path: Path, matrix: list[list[float]]) -> None:
    with out_path.open("w", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(["input_bit", *[f"output_{bit}" for bit in range(OUTPUT_BITS)]])
        for input_bit, row in enumerate(matrix):
            writer.writerow([input_bit, *[f"{value:.6f}" for value in row]])


def avalanche_summary(matrix: list[list[float]], sample_count: int) -> str:
    values = [value for row in matrix for value in row]
    mean_value = sum(values) / len(values)
    mean_abs_bias = sum(abs(value - 0.5) for value in values) / len(values)
    min_value = min(values)
    max_value = max(values)
    worst_cell = max(
        ((abs(value - 0.5), input_bit, output_bit, value) for input_bit, row in enumerate(matrix) for output_bit, value in enumerate(row)),
        key=lambda item: item[0],
    )
    row_biases = [(abs(sum(row) / len(row) - 0.5), idx, sum(row) / len(row)) for idx, row in enumerate(matrix)]
    col_biases = []
    for col in range(OUTPUT_BITS):
        column = [matrix[row][col] for row in range(INPUT_BITS)]
        col_biases.append((abs(sum(column) / len(column) - 0.5), col, sum(column) / len(column)))

    lines = [
        "Jenkins final-mix avalanche test",
        f"samples: {sample_count}",
        f"fixed b: 0x{0x9E3779B9:08X}",
        f"fixed c: 0x{0x243F6A88:08X}",
        "tested input word: a",
        "measured output word: c",
        "",
        f"mean flip probability: {mean_value:.6f}",
        f"mean absolute bias from 50%: {mean_abs_bias:.6f}",
        f"min cell: {min_value:.6f}",
        f"max cell: {max_value:.6f}",
        f"worst cell: input_bit={worst_cell[1]}, output_bit={worst_cell[2]}, value={worst_cell[3]:.6f}",
        "",
        "most biased input rows:",
    ]
    for bias, row_index, row_mean in sorted(row_biases, reverse=True)[:5]:
        lines.append(f"  input_bit {row_index:2d}: row_mean={row_mean:.6f}, bias={bias:.6f}")
    lines.append("most biased output columns:")
    for bias, col_index, col_mean in sorted(col_biases, reverse=True)[:5]:
        lines.append(f"  output_bit {col_index:2d}: col_mean={col_mean:.6f}, bias={bias:.6f}")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description="Problem E hash lab generator")
    parser.add_argument("--out", default="out", help="output directory")
    parser.add_argument("--samples", type=int, default=SAMPLES, help="avalanche sample count")
    args = parser.parse_args()

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    per_constant_counts = [compute_occupancy(constant) for constant in HASH_CONSTANTS]
    stats = [occupancy_stats(counts) for counts in per_constant_counts]

    write_bucket_csv(out_dir / "bucket_occupancy.csv", per_constant_counts)
    write_stats_csv(out_dir / "hash_stats.csv", per_constant_counts, stats)
    write_hash_summary(out_dir / "hash_summary.txt", stats)

    for label, counts in zip(HASH_LABELS, per_constant_counts):
        render_histogram(counts, out_dir / f"hist_{label[2:]}.png")

    matrix = compute_avalanche(args.samples)
    write_avalanche_csv(out_dir / "avalanche_matrix.csv", matrix)
    (out_dir / "avalanche_summary.txt").write_text(avalanche_summary(matrix, args.samples))
    render_heatmap(matrix, out_dir / "avalanche_heatmap.png")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
