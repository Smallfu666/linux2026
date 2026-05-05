# linux2026 quiz4 amei.c experiment report

## Source
- quiz4 URL: `UNKNOWN`
- amei.c source URL: `https://gist.github.com/jserv/221efc83f7b60b996dcf9c8b435980d5/raw/amei.c`
- amei.c sha256: `300cf588f5532c896bcf56deee62ee32a617cbc850a671956cbf0e65ea9ffb36`
- expected upstream sha256: `300cf588f5532c896bcf56deee62ee32a617cbc850a671956cbf0e65ea9ffb36`
- source verification state: `verified_against_expected_upstream`
- generated_at: `2026-05-03T17:47:19Z`
- compiler: `cc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`
- python: `Python 3.14.4`
- uname: `Linux DESKTOP-OK6POFL 6.6.87.2-microsoft-standard-WSL2 #1 SMP PREEMPT_DYNAMIC Thu Jun  5 18:30:46 UTC 2025 x86_64 x86_64 x86_64 GNU/Linux`

## Experiment status table
| ID | Experiment | Status | Main output |
|----|------------|--------|-------------|
| E1 | DCT coefficient recurrence | PASS | `results/dct_coef/summary.txt` |
| E2 | IDCT fixed-point comparison | PASS | `results/idct/summary.txt` |
| E3 | Quadtree block distribution | PASS | `results/coeff_stats/block_distribution_summary.txt` |
| E4 | DCT coefficient statistics | PASS | `results/coeff_stats/coeff_summary.csv` |
| E5 | Compression metrics | PASS | `results/compression/compression_metrics.csv` |
| E6 | Arithmetic context adaptation | PASS | `results/context_model/summary.csv` |
| E7 | Exp-Golomb entropy | PASS | `results/exp_golomb/exp_golomb_entropy.csv` |
| E8 | Filter ablation | PASS | `results/filter_ablation/filter_metrics.csv` |
| E9 | Hex uppercase UB | PASS | `results/ub/hex_uppercase_ub.log` |
| E10 | Arithmetic split overflow | PASS | `results/ub/split_overflow_ub.log` |
| E11 | CLAMP8 side-effect | PASS | `results/ub/clamp8_side_effect.log` |
| E12 | decode_unsigned safe variant | PASS | `results/ub/decode_unsigned_safe_test.log` |
| E13 | CRC nibble vs byte equivalence | PASS | `results/crc/crc_equivalence.txt` |
| E14 | Color transform analysis | SKIPPED | `results/color/status.txt` |
| E15 | Memory layout | PASS | `results/memory/size_output.txt` |

## E1 DCT coefficient recurrence
- command: `make dct`
- output files: `results/dct_coef/dct_coef_compare.csv`, `results/dct_coef/summary.txt`
- key metrics: `max_abs_error=327`, `rmse=171.610979`, `mismatch_count=126`, `recursive_dct_coef_0=1024`, `recursive_dct_coef_1=1446`, `reference_dct_coef_0=1024`, `reference_dct_coef_1=1446`
- observations: the recursive probe tracks the first few coefficients closely, then diverges from the direct cosine reference.
- caveats: `src/amei.original.c` does not contain an explicit `dct_coef[128]` table, so this is a recurrence stand-in probe rather than a direct extract.

## E2 IDCT fixed-point comparison
- command: `make idct`
- output files: `results/idct/idct_error.csv`, `results/idct/summary.txt`
- key metrics: `global_max_abs_error=2`, `mean_rmse=0.203126`, `worst_trial=block_size=8 range_name=large trial=549 max_abs_error=2`
- observations: the fixed-point two-pass transform stayed very close to the floating-point reference on the sampled trials.
- caveats: the probe compares a standalone implementation against a double reference with the same index layout, not a function hook into the original decoder.

## E3 Quadtree block distribution
- command: `make instrument`
- output files: `results/coeff_stats/block_distribution.csv`, `results/coeff_stats/block_distribution_summary.txt`
- key metrics: block size 4 -> leaf_count=216, total_pixels=3456, percentage_of_image=21.093750; block size 8 -> leaf_count=114, total_pixels=7296, percentage_of_image=44.531250; block size 16 -> leaf_count=22, total_pixels=5632, percentage_of_image=34.375000; total_leaf_blocks=352
- observations: this reflects the single embedded bitstream in `amei.c`, not a corpus-wide distribution.

## E4 DCT coefficient statistics
- command: `make instrument`
- output files: `results/coeff_stats/coefficients.csv`, `results/coeff_stats/coeff_summary.csv`
- key metrics: coeff_summary.csv is grouped by channel x block_size x frequency_band; all_rows_nonzero_ratio_1=true; largest_max_abs_level=2240
- caveats: `q` is logged as `NA` because the current instrumentation does not expose a direct quantizer value.

## E5 Compression metrics
- command: `make instrument`
- output files: `results/compression/compression_metrics.csv`, `results/compression/png_sha256.txt`
- key metrics: compressed_bytes=776, image_width=128, image_height=128, pixels=16384, bits_per_pixel=0.37890625, decoded_y_variance=6091.43611358, decoded_cg_variance=19.64110988, decoded_co_variance=554.17757474, residual_coeff_variance_if_available=16475.19649379, nonzero_coeff_ratio=1.00000000
- observations: the instrumented decoder preserves PNG bytes while also collecting coefficient and decoded-plane statistics.

## E6 Arithmetic context adaptation
- command: `make context`
- output files: `results/context_model/context_adaptation.csv`, `results/context_model/summary.csv`
- key metrics: case_a first_step_p1_gt_0_5=97 halving_count=3, case_b first_step_p1_gt_0_5=161 halving_count=7, case_c first_step_p1_gt_0_5=-1 halving_count=3, case_d_p010 first_step_p1_gt_0_5=-1 halving_count=7, case_d_p050 first_step_p1_gt_0_5=1 halving_count=7, case_d_p090 first_step_p1_gt_0_5=1 halving_count=7
- observations: the halving rule shortens effective memory after long runs, and a long zero prefix delays `p1 > 0.5` even after ones start arriving.

## E7 Exp-Golomb entropy
- command: `make exp-golomb`
- output files: `results/exp_golomb/exp_golomb_entropy.csv`
- key metrics: tail_probability stayed below 1e-12 for all scanned p; v_max ranged from 9 to 538; smallest overhead=0.25728823 bits at p=0.45; largest overhead=1.69984826 bits at p=0.05
- caveats: no plot was generated on this machine because `matplotlib` is missing.

## E8 Filter ablation
- command: `make filter-ablation`
- output files: `results/filter_ablation/filter_metrics.csv` and the four generated PNGs
- key metrics: amei_full.png sha256 56017d038fe79ffe49ecd60db5c7b1c433b14b3cb7d0958e05bddb9f80dc021a; amei_none.png sha256 242cc0369234cfce7714481452d419e0c4958a6e2e1f69324ef5716935bed742; amei_deblock_only.png sha256 1ae3cf5c177622dc7ef69735432beaf9066a3fa1e1ba1f9ffe13fbd017005585; amei_bilateral_only.png sha256 b2f17742eb04fea388b75e2873ac462e2a90704f3c89ff3f61526cfe119d52f8; amei_none.png: mean_abs_diff_vs_full=5.438822, max_abs_diff_vs_full=118, boundary_mean_abs_diff=3.030754; amei_deblock_only.png: mean_abs_diff_vs_full=0.982239, max_abs_diff_vs_full=8, boundary_mean_abs_diff=0.881614; amei_bilateral_only.png: mean_abs_diff_vs_full=5.091614, max_abs_diff_vs_full=117, boundary_mean_abs_diff=2.485450
- observations: the deblock-only output stays much closer to full filtering than the bilateral-only or no-filter variants.

## E9 UB findings: hex uppercase / invalid input
- command: `make ub`
- output files: `results/ub/hex_uppercase_ub.log`
- key observations: case=0f returned 0f value=15; case=0F returned 0F value=-17; case=AF returned AF value=-17; case=GG returned GG value=-16; case=f returned f value=-16; uppercase and invalid cases triggered left shift of negative value; empty-input case triggered AddressSanitizer heap-buffer-overflow
- caveats: the probe forks child processes so later cases still run after a sanitizer hit.

## E10 UB findings: arithmetic split overflow
- command: `make ub`
- output files: `results/ub/split_overflow_ub.log`
- key observations: near_intmax_small_obs: safe=715827886, original=-715827879; near_intmax_mid_obs: safe=-1389548243, original=-126322567; realistic: safe=608, original=608; zeroish: safe=8, original=8; overflow sanitizer fired for the near-INT_MAX cases
- caveats: the log records both the safe and original results so overflow-induced divergence is visible directly.

## E11 UB findings: CLAMP8 side-effect
- command: `make ub`
- output files: `results/ub/clamp8_side_effect.log`
- key observations: initial_x=-1 result=0 final_x=0 eval_count=1 interpretation=macro_re_evaluated_operand; initial_x=0 result=2 final_x=3 eval_count=3 interpretation=macro_re_evaluated_operand; initial_x=128 result=130 final_x=131 eval_count=3 interpretation=macro_re_evaluated_operand; initial_x=255 result=255 final_x=257 eval_count=2 interpretation=macro_re_evaluated_operand; initial_x=256 result=255 final_x=258 eval_count=2 interpretation=macro_re_evaluated_operand
- caveats: this demonstrates macro operand re-evaluation risk, not automatic UB by itself.

## E12 UB findings: decode_unsigned safe variant
- command: `make ub`
- output files: `results/ub/decode_unsigned_safe_test.log`
- key observations: normal_v_0: safe=0, original=0; normal_v_1: safe=1, original=1; normal_v_15: safe=15, original=15; long_prefix_31: safe=-1, original=-2; long_prefix_64: safe=-2, original=-2; eof_before_terminator: safe=-1, original=14; original decoder hit signed-left-shift UB on long prefixes
- caveats: the safe variant uses explicit negative return codes to distinguish checked failure modes.

## E13 CRC nibble vs byte equivalence
- command: `make crc`
- output files: `results/crc/crc_equivalence.txt`
- key metrics: `all_match=true`, `tested_states=10003`, `tested_bytes=2560768`, `nibble_table_bytes=64`, `byte_table_bytes=1024`, `first_mismatch_if_any=none`
- observations: the nibble-table and byte-table updates matched across the tested states and all byte values.

## E14 Color transform analysis
- command: `make color`
- output files: `results/color/status.txt`
- status: `SKIPPED: missing numpy`
- caveats: this machine currently lacks `numpy`, so the color-analysis CSVs are skipped rather than fabricated.

## E15 Memory layout
- command: `make memory`
- output files: `results/memory/size_output.txt`, `results/memory/ulimit_stack.txt`, `results/memory/static_run.log`, `results/memory/stack_run.log`, `results/memory/proc_maps.txt`
- key metrics: size_output.txt reports bss=8388672; global_static_bytes=4194304; stack_log_present=true; proc_maps_has_stack_mapping=true
- caveats: this is a user-space process layout probe, not a kernel memory-map experiment.

## Files generated
- `Makefile`
- `README.md`
- `REPORT.md`
- `docs/superpowers/plans/2026-05-04-linux2026-quiz4-amei-lab.md`
- `results/coeff_stats/block_distribution.csv`
- `results/coeff_stats/block_distribution_summary.txt`
- `results/coeff_stats/coeff_summary.csv`
- `results/coeff_stats/coefficients.csv`
- `results/color/status.txt`
- `results/compression/amei_instrumented`
- `results/compression/amei_instrumented.png`
- `results/compression/amei_original`
- `results/compression/amei_original.png`
- `results/compression/compression_metrics.csv`
- `results/compression/png_sha256.txt`
- `results/context_model/context_adaptation.csv`
- `results/context_model/simulate_context_model`
- `results/context_model/summary.csv`
- `results/crc/crc_equivalence.txt`
- `results/crc/crc_nibble_vs_byte`
- `results/dct_coef/dct_coef_compare.csv`
- `results/dct_coef/summary.txt`
- `results/dct_coef/verify_dct_coef`
- `results/exp_golomb/exp_golomb_entropy.csv`
- `results/filter_ablation/amei_bilateral_only`
- `results/filter_ablation/amei_bilateral_only.png`
- `results/filter_ablation/amei_deblock_only`
- `results/filter_ablation/amei_deblock_only.png`
- `results/filter_ablation/amei_full`
- `results/filter_ablation/amei_full.png`
- `results/filter_ablation/amei_none`
- `results/filter_ablation/amei_none.png`
- `results/filter_ablation/filter_metrics.csv`
- `results/idct/compare_idct`
- `results/idct/idct_error.csv`
- `results/idct/summary.txt`
- `results/memory/memory_layout_test`
- `results/memory/proc_maps.txt`
- `results/memory/size_output.txt`
- `results/memory/stack_run.log`
- `results/memory/static_run.log`
- `results/memory/ulimit_stack.txt`
- `results/source_manifest.txt`
- `results/source_sha256.txt`
- `results/ub/clamp8_side_effect`
- `results/ub/clamp8_side_effect.log`
- `results/ub/decode_unsigned_safe_test`
- `results/ub/decode_unsigned_safe_test.log`
- `results/ub/hex_uppercase_ub`
- `results/ub/hex_uppercase_ub.log`
- `results/ub/split_overflow_ub.O0`
- `results/ub/split_overflow_ub.O2`
- `results/ub/split_overflow_ub.log`
- `scripts/__pycache__/png_metrics.cpython-314.pyc`
- `scripts/__pycache__/summarize_results.cpython-314.pyc`
- `scripts/fetch_amei.sh`
- `scripts/png_metrics.py`
- `scripts/run_all.sh`
- `scripts/summarize_results.py`
- `src/__pycache__/color_matrix_analysis.cpython-314.pyc`
- `src/amei.instrumented.c`
- `src/amei.original.c`
- `src/color_matrix_analysis.py`
- `src/compare_idct.c`
- `src/crc_nibble_vs_byte.c`
- `src/exp_golomb_stats.py`
- `src/memory_layout_test.c`
- `src/simulate_context_model.c`
- `src/verify_dct_coef.c`
- `ub_tests/clamp8_side_effect.c`
- `ub_tests/decode_unsigned_safe_test.c`
- `ub_tests/hex_uppercase_ub.c`
- `ub_tests/split_overflow_ub.c`
