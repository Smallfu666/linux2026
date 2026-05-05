#!/usr/bin/env python3
import csv
import hashlib
import os
import struct
import sys
import zlib


def read_png(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path}: not a PNG")
    pos = 8
    width = height = None
    bit_depth = color_type = None
    idat = bytearray()
    while pos < len(data):
        length = struct.unpack(">I", data[pos:pos + 4])[0]
        pos += 4
        ctype = data[pos:pos + 4]
        pos += 4
        chunk = data[pos:pos + length]
        pos += length + 4
        if ctype == b"IHDR":
            width, height, bit_depth, color_type, _, _, _ = struct.unpack(">IIBBBBB", chunk)
        elif ctype == b"IDAT":
            idat.extend(chunk)
        elif ctype == b"IEND":
            break
    raw = zlib.decompress(bytes(idat))
    if color_type != 2 or bit_depth != 8:
        raise ValueError(f"{path}: unsupported PNG format {bit_depth=} {color_type=}")
    stride = width * 3
    rows = []
    offset = 0
    prev = bytearray(stride)
    for _ in range(height):
        filt = raw[offset]
        offset += 1
        row = bytearray(raw[offset:offset + stride])
        offset += stride
        if filt == 0:
            pass
        elif filt == 1:
            for i in range(stride):
                left = row[i - 3] if i >= 3 else 0
                row[i] = (row[i] + left) & 0xFF
        elif filt == 2:
            for i in range(stride):
                row[i] = (row[i] + prev[i]) & 0xFF
        elif filt == 3:
            for i in range(stride):
                left = row[i - 3] if i >= 3 else 0
                up = prev[i]
                row[i] = (row[i] + ((left + up) >> 1)) & 0xFF
        elif filt == 4:
            for i in range(stride):
                a = row[i - 3] if i >= 3 else 0
                b = prev[i]
                c = prev[i - 3] if i >= 3 else 0
                p = a + b - c
                pa = abs(p - a)
                pb = abs(p - b)
                pc = abs(p - c)
                pr = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                row[i] = (row[i] + pr) & 0xFF
        else:
            raise ValueError(f"{path}: unknown filter {filt}")
        rows.append(bytes(row))
        prev = row
    return width, height, b"".join(rows)


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def main(argv):
    if len(argv) < 2:
        print("usage: png_metrics.py FULL.png VARIANT.png ...", file=sys.stderr)
        return 2
    full_path = argv[1]
    fw, fh, fraw = read_png(full_path)
    full_sha = sha256(full_path)
    writer = csv.writer(sys.stdout)
    writer.writerow(["variant", "mean_abs_diff_vs_full", "max_abs_diff_vs_full", "boundary_mean_abs_diff"])
    for path in argv[1:]:
        w, h, raw = read_png(path)
        if (w, h) != (fw, fh):
            raise ValueError(f"{path}: dimensions {w}x{h} != {fw}x{fh}")
        diffs = [abs(a - b) for a, b in zip(raw, fraw)]
        mean_abs = sum(diffs) / len(diffs) if diffs else 0.0
        max_abs = max(diffs) if diffs else 0
        boundary = 0
        boundary_n = 0
        for y in range(h):
            for x in range(w):
                if x in (0, 1, w - 2, w - 1) or y in (0, 1, h - 2, h - 1):
                    idx = (y * w + x) * 3
                    boundary += sum(abs(raw[idx + c] - fraw[idx + c]) for c in range(3))
                    boundary_n += 3
        boundary_mean = boundary / boundary_n if boundary_n else 0.0
        writer.writerow([os.path.basename(path), f"{mean_abs:.6f}", str(max_abs), f"{boundary_mean:.6f}"])
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
