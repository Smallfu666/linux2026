# linux2026 quiz5 labs

這組 lab 用來輔助理解第 5 週測驗的 Part 2 與延伸問題。重點不是取代文字作答，而是提供可重跑的小實驗，確認位元操作、未定義行為、寬乘法與 IEEE 754 捨入邊界。

## 題目判斷

| 題目 | 是否值得實作 | 原因 |
| --- | --- | --- |
| Problem A | 是 | `replace_nbytes`、`GENMASK`、`FIELD_PREP` 都是位移與遮罩邊界問題，實作可直接驗證 A01/A02/A04 與 `n == 4` 特例。 |
| Problem B | 必要 | signed shift UB、`BIT(n)`、strict aliasing 都會受編譯器最佳化影響，必須看 `-O0`/`-O2` assembly 與 sanitizer 才能建立直覺。 |
| Problem C | 是 | `unsigned_high_prod` 修正式與 `mul_u64_u32_shr` fallback 都適合用 `__int128` oracle 驗證。 |
| Problem D | 必要 | fp32 to fp16 的 denormal boundary、round-to-even、NaN/Inf 都有很多邊界案例，純整數 reference 測試最有價值。 |
| Problem E | 必要 | `float_quarter` 的 `exp in {0,1,2}` 非對稱位移與進位回最小 normalized 需要實驗確認。 |

## Layout

- `labs/a_bitfield`: Problem A，byte/field replacement、mask、GENMASK-like checks。
- `labs/b_ub_alias`: Problem B，shift UB、`BIT()`、strict aliasing assembly probes。
- `labs/c_mul_high`: Problem C，高位乘積修正式、partial-product 寬乘法、overflow helper。
- `labs/d_fp16`: Problem D，fp32 to fp16 純整數轉換與捨入驗證。
- `labs/e_float_quarter`: Problem E，float quarter、denormalized path、round-to-even。

## Reproducible Commands

```sh
make test
make clean
```

各子目錄也可單獨執行：

```sh
make -C labs/a_bitfield test
make -C labs/b_ub_alias test
make -C labs/c_mul_high test
make -C labs/d_fp16 test
make -C labs/e_float_quarter test
```

## Fill-In Anchors

這些值只作為 lab 驗證錨點，正式作答仍應搭配課程教材、C99 條文、Linux source/git log 說明。

| Blank | Value |
| --- | --- |
| A01 | `mask<<shift` |
| A02 | `mask` |
| A03 | `undefined behavior` |
| A04 | `AA112233` |
| B01 | `wrapped` |
| B02 | `undefined behavior` |
| C01 | `x` |
| C02 | `FFFFFFFE` |
| D01 | `112` |
| D02 | `14` |
| D03 | `12` |
| E01 | `2` |
| E02 | `1` |
| E03 | `00200000` |
