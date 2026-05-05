#!/usr/bin/env python3
import csv
import sys
from pathlib import Path


def load_csv(path: Path):
    raw = {}
    lemire = {}
    with path.open(newline="") as fh:
        reader = csv.DictReader(fh)
        for row in reader:
            bucket = int(row["value"])
            count = int(row["count"])
            if row["method"] == "raw":
                raw[bucket] = count
            else:
                lemire[bucket] = count
    keys = sorted(set(raw) | set(lemire))
    return keys, [raw.get(k, 0) for k in keys], [lemire.get(k, 0) for k in keys]


def main():
    in_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("results/modulo_bias.csv")
    out_path = Path(sys.argv[2]) if len(sys.argv) > 2 else in_path.with_suffix(".png")
    keys, raw, lemire = load_csv(in_path)

    width = 12.0
    height = 6.0
    margin = 0.5
    plot_w = width - 2 * margin
    plot_h = height - 2 * margin
    max_count = max(max(raw, default=1), max(lemire, default=1))
    bar_count = len(keys)
    group_w = plot_w / max(bar_count, 1)
    bar_w = group_w * 0.4

    def y_scale(value):
        return margin + plot_h - (value / max_count) * plot_h

    def x_scale(i, offset):
        return margin + i * group_w + offset

    svg = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width * 100}" height="{height * 100}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="white"/>',
        f'<text x="{margin}" y="{0.3}" font-size="0.22">Modulo Bias Distribution</text>',
        f'<line x1="{margin}" y1="{margin}" x2="{margin}" y2="{margin + plot_h}" stroke="black" stroke-width="0.02"/>',
        f'<line x1="{margin}" y1="{margin + plot_h}" x2="{margin + plot_w}" y2="{margin + plot_h}" stroke="black" stroke-width="0.02"/>',
    ]

    for i, key in enumerate(keys):
        raw_h = (raw[i] / max_count) * plot_h
        lemire_h = (lemire[i] / max_count) * plot_h
        x0 = x_scale(i, group_w * 0.1)
        x1 = x_scale(i, group_w * 0.55)
        y0 = margin + plot_h - raw_h
        y1 = margin + plot_h - lemire_h
        svg.append(f'<rect x="{x0:.3f}" y="{y0:.3f}" width="{bar_w:.3f}" height="{raw_h:.3f}" fill="#4c78a8"/>')
        svg.append(f'<rect x="{x1:.3f}" y="{y1:.3f}" width="{bar_w:.3f}" height="{lemire_h:.3f}" fill="#f58518"/>')
        if i % max(1, len(keys) // 12) == 0:
            svg.append(f'<text x="{x0:.3f}" y="{height - 0.15}" font-size="0.12">{key}</text>')

    svg.extend([
        '<rect x="9.2" y="0.7" width="0.18" height="0.18" fill="#4c78a8"/>',
        '<text x="9.45" y="0.84" font-size="0.14">raw % n</text>',
        '<rect x="9.2" y="0.98" width="0.18" height="0.18" fill="#f58518"/>',
        '<text x="9.45" y="1.12" font-size="0.14">lemire</text>',
        '</svg>',
    ])

    out_path.write_text("\n".join(svg), encoding="utf-8")
    print(f"wrote {out_path}")


if __name__ == "__main__":
    main()
