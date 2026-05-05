#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
RESULTS = ROOT / "results" / "color"


def main() -> int:
    RESULTS.mkdir(parents=True, exist_ok=True)
    status_path = RESULTS / "status.txt"
    try:
        import numpy as np  # type: ignore
    except Exception:
        status_path.write_text("SKIPPED: missing numpy\n")
        return 0

    matrices = {
        "YCgCo_forward": np.array([[0.25, 0.5, 0.25], [-0.25, 0.5, -0.25], [0.5, 0.0, -0.5]], dtype=float),
        "YCgCo_inverse": np.array([[1.0, -1.0, 1.0], [1.0, 1.0, 0.0], [1.0, -1.0, -1.0]], dtype=float),
        "BT601_rgb_to_ycbcr": np.array([[0.299, 0.587, 0.114], [-0.168736, -0.331264, 0.5], [0.5, -0.418688, -0.081312]], dtype=float),
    }

    with (RESULTS / "color_matrix_metrics.csv").open("w") as f:
        f.write("matrix_name,cond2\n")
        for name, mat in matrices.items():
            cond = float(np.linalg.cond(mat, 2))
            f.write(f"{name},{cond:.8f}\n")

    with (RESULTS / "fixed_point_error.csv").open("w") as f:
        f.write("matrix_name,coef_name,real_value,q8_value,q8_error,q10_value,q10_error\n")
        for name, mat in matrices.items():
            for row in range(mat.shape[0]):
                for col in range(mat.shape[1]):
                    value = float(mat[row, col])
                    q8 = round(value * 256.0) / 256.0
                    q10 = round(value * 1024.0) / 1024.0
                    f.write(
                        f"{name},r{row}c{col},{value:.8f},{q8:.8f},{abs(q8 - value):.8f},{q10:.8f},{abs(q10 - value):.8f}\n"
                    )
    status_path.write_text("PASS: generated color_matrix_metrics.csv and fixed_point_error.csv\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
