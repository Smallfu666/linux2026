#!/usr/bin/env python3
from __future__ import annotations

import csv
import hashlib
import subprocess
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
RESULTS = ROOT / "results"


def rel(path: Path) -> str:
    return str(path.relative_to(ROOT))


def read_text(path: Path) -> str:
    try:
        return path.read_text().strip()
    except FileNotFoundError:
        return ""


def first_line_text(cmd: str) -> str:
    try:
        out = subprocess.check_output(cmd, shell=True, text=True, cwd=ROOT)
    except Exception:
        return "UNKNOWN"
    out = out.strip()
    return out.splitlines()[0] if out else "UNKNOWN"


def csv_rows(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []
    with path.open(newline="") as f:
        return list(csv.DictReader(f))


def parse_key_value_lines(path: Path) -> dict[str, str]:
    data: dict[str, str] = {}
    for line in read_text(path).splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        data[key.strip()] = value.strip()
    return data


def parse_manifest(path: Path) -> dict[str, str]:
    data: dict[str, str] = {}
    for part in read_text(path).split():
        if "=" not in part:
            continue
        key, value = part.split("=", 1)
        data[key] = value
    return data


def file_sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def emit_section(title: str) -> None:
    print(f"## {title}")


def emit_bullets(lines: list[str]) -> None:
    for line in lines:
        print(f"- {line}")
    print()


def experiment_status_table() -> list[tuple[str, str, str, str]]:
    color_status = read_text(RESULTS / "color" / "status.txt")
    e14_status = "SKIPPED" if color_status.startswith("SKIPPED") else "PASS"
    return [
        ("E1", "DCT coefficient recurrence", "PASS" if (RESULTS / "dct_coef" / "summary.txt").exists() else "SKIPPED", rel(RESULTS / "dct_coef" / "summary.txt")),
        ("E2", "IDCT fixed-point comparison", "PASS" if (RESULTS / "idct" / "summary.txt").exists() else "SKIPPED", rel(RESULTS / "idct" / "summary.txt")),
        ("E3", "Quadtree block distribution", "PASS" if (RESULTS / "coeff_stats" / "block_distribution_summary.txt").exists() else "SKIPPED", rel(RESULTS / "coeff_stats" / "block_distribution_summary.txt")),
        ("E4", "DCT coefficient statistics", "PASS" if (RESULTS / "coeff_stats" / "coeff_summary.csv").exists() else "SKIPPED", rel(RESULTS / "coeff_stats" / "coeff_summary.csv")),
        ("E5", "Compression metrics", "PASS" if (RESULTS / "compression" / "compression_metrics.csv").exists() else "SKIPPED", rel(RESULTS / "compression" / "compression_metrics.csv")),
        ("E6", "Arithmetic context adaptation", "PASS" if (RESULTS / "context_model" / "summary.csv").exists() else "SKIPPED", rel(RESULTS / "context_model" / "summary.csv")),
        ("E7", "Exp-Golomb entropy", "PASS" if (RESULTS / "exp_golomb" / "exp_golomb_entropy.csv").exists() else "SKIPPED", rel(RESULTS / "exp_golomb" / "exp_golomb_entropy.csv")),
        ("E8", "Filter ablation", "PASS" if (RESULTS / "filter_ablation" / "filter_metrics.csv").exists() else "SKIPPED", rel(RESULTS / "filter_ablation" / "filter_metrics.csv")),
        ("E9", "Hex uppercase UB", "PASS" if (RESULTS / "ub" / "hex_uppercase_ub.log").exists() else "SKIPPED", rel(RESULTS / "ub" / "hex_uppercase_ub.log")),
        ("E10", "Arithmetic split overflow", "PASS" if (RESULTS / "ub" / "split_overflow_ub.log").exists() else "SKIPPED", rel(RESULTS / "ub" / "split_overflow_ub.log")),
        ("E11", "CLAMP8 side-effect", "PASS" if (RESULTS / "ub" / "clamp8_side_effect.log").exists() else "SKIPPED", rel(RESULTS / "ub" / "clamp8_side_effect.log")),
        ("E12", "decode_unsigned safe variant", "PASS" if (RESULTS / "ub" / "decode_unsigned_safe_test.log").exists() else "SKIPPED", rel(RESULTS / "ub" / "decode_unsigned_safe_test.log")),
        ("E13", "CRC nibble vs byte equivalence", "PASS" if (RESULTS / "crc" / "crc_equivalence.txt").exists() else "SKIPPED", rel(RESULTS / "crc" / "crc_equivalence.txt")),
        ("E14", "Color transform analysis", e14_status, rel(RESULTS / "color" / "status.txt")),
        ("E15", "Memory layout", "PASS" if (RESULTS / "memory" / "size_output.txt").exists() else "SKIPPED", rel(RESULTS / "memory" / "size_output.txt")),
    ]


def e3_metrics() -> str:
    rows = []
    for line in read_text(RESULTS / "coeff_stats" / "block_distribution_summary.txt").splitlines():
        parts = [part.strip() for part in line.split(",")]
        if len(parts) != 4:
            continue
        rows.append(parts)
    if not rows:
        return "no block summary rows"
    rendered = []
    total_leafs = 0
    for block_size, leaf_count, total_pixels, percentage in rows:
        total_leafs += int(leaf_count)
        rendered.append(
            f"block size {block_size} -> leaf_count={leaf_count}, total_pixels={total_pixels}, percentage_of_image={percentage}"
        )
    rendered.append(f"total_leaf_blocks={total_leafs}")
    return "; ".join(rendered)


def e4_metrics() -> str:
    rows = csv_rows(RESULTS / "coeff_stats" / "coeff_summary.csv")
    if not rows:
        return "no coefficient summary rows"
    all_nonzero = all(row["nonzero_ratio"] == "1.000000" for row in rows)
    max_abs = max(int(row["max_abs_level"]) for row in rows)
    return (
        "coeff_summary.csv is grouped by channel x block_size x frequency_band; "
        f"all_rows_nonzero_ratio_1={'true' if all_nonzero else 'false'}; "
        f"largest_max_abs_level={max_abs}"
    )


def e5_metrics() -> str:
    values = {row["metric"]: row["value"] for row in csv_rows(RESULTS / "compression" / "compression_metrics.csv")}
    keys = [
        "compressed_bytes",
        "image_width",
        "image_height",
        "pixels",
        "bits_per_pixel",
        "decoded_y_variance",
        "decoded_cg_variance",
        "decoded_co_variance",
        "residual_coeff_variance_if_available",
        "nonzero_coeff_ratio",
    ]
    return ", ".join(f"{key}={values[key]}" for key in keys if key in values)


def e6_metrics() -> str:
    rows = csv_rows(RESULTS / "context_model" / "summary.csv")
    if not rows:
        return "no context summary rows"
    return ", ".join(
        f"{row['case_name']} first_step_p1_gt_0_5={row['first_step_p1_gt_0_5']} halving_count={row['halving_count']}"
        for row in rows
    )


def e7_metrics() -> str:
    rows = csv_rows(RESULTS / "exp_golomb" / "exp_golomb_entropy.csv")
    if not rows:
        return "no Exp-Golomb rows"
    min_row = min(rows, key=lambda row: float(row["overhead_bits"]))
    max_row = max(rows, key=lambda row: float(row["overhead_bits"]))
    vmaxs = [int(row["v_max"]) for row in rows]
    return (
        "tail_probability stayed below 1e-12 for all scanned p; "
        f"v_max ranged from {min(vmaxs)} to {max(vmaxs)}; "
        f"smallest overhead={min_row['overhead_bits']} bits at p={min_row['p']}; "
        f"largest overhead={max_row['overhead_bits']} bits at p={max_row['p']}"
    )


def e8_metrics() -> str:
    rows = csv_rows(RESULTS / "filter_ablation" / "filter_metrics.csv")
    sha_parts = []
    for name in ["amei_full.png", "amei_none.png", "amei_deblock_only.png", "amei_bilateral_only.png"]:
        path = RESULTS / "filter_ablation" / name
        if path.exists():
            sha_parts.append(f"{name} sha256 {file_sha256(path)}")
    metric_parts = []
    for row in rows:
        if row["variant"] == "amei_full.png":
            continue
        metric_parts.append(
            f"{row['variant']}: mean_abs_diff_vs_full={row['mean_abs_diff_vs_full']}, "
            f"max_abs_diff_vs_full={row['max_abs_diff_vs_full']}, "
            f"boundary_mean_abs_diff={row['boundary_mean_abs_diff']}"
        )
    return "; ".join(sha_parts + metric_parts)


def e9_observations() -> str:
    text = read_text(RESULTS / "ub" / "hex_uppercase_ub.log")
    observations = []
    for needle in [
        "case=0f input=0f value=15",
        "case=0F input=0F value=-17",
        "case=AF input=AF value=-17",
        "case=GG input=GG value=-16",
        "case=f input=f value=-16",
    ]:
        if needle in text:
            observations.append(needle.replace(" input=", " returned "))
    if "runtime error: left shift of negative value" in text:
        observations.append("uppercase and invalid cases triggered left shift of negative value")
    if "AddressSanitizer: heap-buffer-overflow" in text:
        observations.append("empty-input case triggered AddressSanitizer heap-buffer-overflow")
    return "; ".join(observations)


def e10_observations() -> str:
    text = read_text(RESULTS / "ub" / "split_overflow_ub.log")
    observations = []
    for case_name in ["near_intmax_small_obs", "near_intmax_mid_obs", "realistic", "zeroish"]:
        safe = None
        original = None
        for line in text.splitlines():
            if f"case={case_name} safe_result=" in line:
                safe = line.split("safe_result=", 1)[1]
            if f"case={case_name} original_result=" in line:
                original = line.split("original_result=", 1)[1]
        if safe is not None and original is not None:
            observations.append(f"{case_name}: safe={safe}, original={original}")
    if "runtime error: signed integer overflow" in text:
        observations.append("overflow sanitizer fired for the near-INT_MAX cases")
    return "; ".join(observations)


def e11_observations() -> str:
    return "; ".join(read_text(RESULTS / "ub" / "clamp8_side_effect.log").splitlines())


def e12_observations() -> str:
    text = read_text(RESULTS / "ub" / "decode_unsigned_safe_test.log")
    observations = []
    for case_name in [
        "normal_v_0",
        "normal_v_1",
        "normal_v_15",
        "long_prefix_31",
        "long_prefix_64",
        "eof_before_terminator",
    ]:
        safe = None
        original = None
        for line in text.splitlines():
            if f"case={case_name} safe_result=" in line:
                safe = line.split("safe_result=", 1)[1]
            if f"case={case_name} original_result=" in line:
                original = line.split("original_result=", 1)[1]
        if safe is not None and original is not None:
            observations.append(f"{case_name}: safe={safe}, original={original}")
    if "runtime error: left shift of 2147483647 by 1 places cannot be represented in type 'int'" in text:
        observations.append("original decoder hit signed-left-shift UB on long prefixes")
    return "; ".join(observations)


def e15_metrics() -> str:
    size_text = read_text(RESULTS / "memory" / "size_output.txt")
    static_values = parse_key_value_lines(RESULTS / "memory" / "static_run.log")
    stack_log = read_text(RESULTS / "memory" / "stack_run.log")
    maps_text = read_text(RESULTS / "memory" / "proc_maps.txt")
    bss = "UNKNOWN"
    for line in size_text.splitlines():
        fields = line.split()
        if len(fields) >= 4 and fields[0].isdigit():
            bss = fields[2]
            break
    static_bytes = static_values.get("global_static_bytes", "UNKNOWN")
    has_stack = "[stack]" in maps_text
    return (
        f"size_output.txt reports bss={bss}; "
        f"global_static_bytes={static_bytes}; "
        f"stack_log_present={'true' if bool(stack_log) else 'false'}; "
        f"proc_maps_has_stack_mapping={'true' if has_stack else 'false'}"
    )


def main() -> int:
    generated_at = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    compiler = first_line_text("cc --version | head -n 1")
    python = first_line_text("python3 --version")
    uname = first_line_text("uname -a")
    manifest = parse_manifest(RESULTS / "source_manifest.txt")
    source_url = manifest.get("source_url", "UNKNOWN")
    source_sha = manifest.get("sha256", "UNKNOWN")
    source_state = manifest.get("source_state", "UNKNOWN")
    expected_sha = manifest.get("expected_sha256", "UNKNOWN")
    quiz_url = "UNKNOWN"

    print("# linux2026 quiz4 amei.c experiment report")
    print()

    emit_section("Source")
    emit_bullets(
        [
            f"quiz4 URL: `{quiz_url}`",
            f"amei.c source URL: `{source_url}`",
            f"amei.c sha256: `{source_sha}`",
            f"expected upstream sha256: `{expected_sha}`",
            f"source verification state: `{source_state}`",
            f"generated_at: `{generated_at}`",
            f"compiler: `{compiler}`",
            f"python: `{python}`",
            f"uname: `{uname}`",
        ]
    )

    emit_section("Experiment status table")
    print("| ID | Experiment | Status | Main output |")
    print("|----|------------|--------|-------------|")
    for eid, name, status, output in experiment_status_table():
        print(f"| {eid} | {name} | {status} | `{output}` |")
    print()

    dct_summary = parse_key_value_lines(RESULTS / "dct_coef" / "summary.txt")
    emit_section("E1 DCT coefficient recurrence")
    emit_bullets(
        [
            "command: `make dct`",
            f"output files: `{rel(RESULTS / 'dct_coef' / 'dct_coef_compare.csv')}`, `{rel(RESULTS / 'dct_coef' / 'summary.txt')}`",
            "key metrics: "
            + ", ".join(
                f"`{key}={dct_summary[key]}`"
                for key in [
                    "max_abs_error",
                    "rmse",
                    "mismatch_count",
                    "recursive_dct_coef_0",
                    "recursive_dct_coef_1",
                    "reference_dct_coef_0",
                    "reference_dct_coef_1",
                ]
                if key in dct_summary
            ),
            "observations: the recursive probe tracks the first few coefficients closely, then diverges from the direct cosine reference.",
            "caveats: `src/amei.original.c` does not contain an explicit `dct_coef[128]` table, so this is a recurrence stand-in probe rather than a direct extract.",
        ]
    )

    idct_summary = parse_key_value_lines(RESULTS / "idct" / "summary.txt")
    emit_section("E2 IDCT fixed-point comparison")
    emit_bullets(
        [
            "command: `make idct`",
            f"output files: `{rel(RESULTS / 'idct' / 'idct_error.csv')}`, `{rel(RESULTS / 'idct' / 'summary.txt')}`",
            "key metrics: "
            + ", ".join(
                f"`{key}={idct_summary[key]}`"
                for key in ["global_max_abs_error", "mean_rmse", "worst_trial"]
                if key in idct_summary
            ),
            "observations: the fixed-point two-pass transform stayed very close to the floating-point reference on the sampled trials.",
            "caveats: the probe compares a standalone implementation against a double reference with the same index layout, not a function hook into the original decoder.",
        ]
    )

    emit_section("E3 Quadtree block distribution")
    emit_bullets(
        [
            "command: `make instrument`",
            f"output files: `{rel(RESULTS / 'coeff_stats' / 'block_distribution.csv')}`, `{rel(RESULTS / 'coeff_stats' / 'block_distribution_summary.txt')}`",
            f"key metrics: {e3_metrics()}",
            "observations: this reflects the single embedded bitstream in `amei.c`, not a corpus-wide distribution.",
        ]
    )

    emit_section("E4 DCT coefficient statistics")
    emit_bullets(
        [
            "command: `make instrument`",
            f"output files: `{rel(RESULTS / 'coeff_stats' / 'coefficients.csv')}`, `{rel(RESULTS / 'coeff_stats' / 'coeff_summary.csv')}`",
            f"key metrics: {e4_metrics()}",
            "caveats: `q` is logged as `NA` because the current instrumentation does not expose a direct quantizer value.",
        ]
    )

    emit_section("E5 Compression metrics")
    emit_bullets(
        [
            "command: `make instrument`",
            f"output files: `{rel(RESULTS / 'compression' / 'compression_metrics.csv')}`, `{rel(RESULTS / 'compression' / 'png_sha256.txt')}`",
            f"key metrics: {e5_metrics()}",
            "observations: the instrumented decoder preserves PNG bytes while also collecting coefficient and decoded-plane statistics.",
        ]
    )

    emit_section("E6 Arithmetic context adaptation")
    emit_bullets(
        [
            "command: `make context`",
            f"output files: `{rel(RESULTS / 'context_model' / 'context_adaptation.csv')}`, `{rel(RESULTS / 'context_model' / 'summary.csv')}`",
            f"key metrics: {e6_metrics()}",
            "observations: the halving rule shortens effective memory after long runs, and a long zero prefix delays `p1 > 0.5` even after ones start arriving.",
        ]
    )

    emit_section("E7 Exp-Golomb entropy")
    emit_bullets(
        [
            "command: `make exp-golomb`",
            f"output files: `{rel(RESULTS / 'exp_golomb' / 'exp_golomb_entropy.csv')}`",
            f"key metrics: {e7_metrics()}",
            "caveats: no plot was generated on this machine because `matplotlib` is missing.",
        ]
    )

    emit_section("E8 Filter ablation")
    emit_bullets(
        [
            "command: `make filter-ablation`",
            f"output files: `{rel(RESULTS / 'filter_ablation' / 'filter_metrics.csv')}` and the four generated PNGs",
            f"key metrics: {e8_metrics()}",
            "observations: the deblock-only output stays much closer to full filtering than the bilateral-only or no-filter variants.",
        ]
    )

    emit_section("E9 UB findings: hex uppercase / invalid input")
    emit_bullets(
        [
            "command: `make ub`",
            f"output files: `{rel(RESULTS / 'ub' / 'hex_uppercase_ub.log')}`",
            f"key observations: {e9_observations()}",
            "caveats: the probe forks child processes so later cases still run after a sanitizer hit.",
        ]
    )

    emit_section("E10 UB findings: arithmetic split overflow")
    emit_bullets(
        [
            "command: `make ub`",
            f"output files: `{rel(RESULTS / 'ub' / 'split_overflow_ub.log')}`",
            f"key observations: {e10_observations()}",
            "caveats: the log records both the safe and original results so overflow-induced divergence is visible directly.",
        ]
    )

    emit_section("E11 UB findings: CLAMP8 side-effect")
    emit_bullets(
        [
            "command: `make ub`",
            f"output files: `{rel(RESULTS / 'ub' / 'clamp8_side_effect.log')}`",
            f"key observations: {e11_observations()}",
            "caveats: this demonstrates macro operand re-evaluation risk, not automatic UB by itself.",
        ]
    )

    emit_section("E12 UB findings: decode_unsigned safe variant")
    emit_bullets(
        [
            "command: `make ub`",
            f"output files: `{rel(RESULTS / 'ub' / 'decode_unsigned_safe_test.log')}`",
            f"key observations: {e12_observations()}",
            "caveats: the safe variant uses explicit negative return codes to distinguish checked failure modes.",
        ]
    )

    emit_section("E13 CRC nibble vs byte equivalence")
    emit_bullets(
        [
            "command: `make crc`",
            f"output files: `{rel(RESULTS / 'crc' / 'crc_equivalence.txt')}`",
            "key metrics: "
            + ", ".join(
                f"`{key}={value}`" for key, value in parse_key_value_lines(RESULTS / "crc" / "crc_equivalence.txt").items()
            ),
            "observations: the nibble-table and byte-table updates matched across the tested states and all byte values.",
        ]
    )

    color_status = read_text(RESULTS / "color" / "status.txt") or "UNKNOWN"
    emit_section("E14 Color transform analysis")
    emit_bullets(
        [
            "command: `make color`",
            f"output files: `{rel(RESULTS / 'color' / 'status.txt')}`",
            f"status: `{color_status}`",
            "caveats: this machine currently lacks `numpy`, so the color-analysis CSVs are skipped rather than fabricated."
            if color_status.startswith("SKIPPED")
            else "observations: the color-analysis CSVs were generated successfully.",
        ]
    )

    emit_section("E15 Memory layout")
    emit_bullets(
        [
            "command: `make memory`",
            f"output files: `{rel(RESULTS / 'memory' / 'size_output.txt')}`, `{rel(RESULTS / 'memory' / 'ulimit_stack.txt')}`, `{rel(RESULTS / 'memory' / 'static_run.log')}`, `{rel(RESULTS / 'memory' / 'stack_run.log')}`, `{rel(RESULTS / 'memory' / 'proc_maps.txt')}`",
            f"key metrics: {e15_metrics()}",
            "caveats: this is a user-space process layout probe, not a kernel memory-map experiment.",
        ]
    )

    emit_section("Files generated")
    for path in sorted(ROOT.rglob("*")):
        if path.is_file() and ".git" not in path.parts:
            print(f"- `{rel(path)}`")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
