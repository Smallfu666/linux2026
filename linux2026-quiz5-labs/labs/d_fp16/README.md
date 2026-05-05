# Problem D: float32_to_float16

這個實驗用純整數位元操作實作 `float32_to_float16`，重點放在：

- denormalized 邊界
- guard / round / sticky
- round-to-even
- NaN / Inf / signed zero

## 執行方式

在此目錄內：

```sh
make test
```

## 測試設計

程式內含三條路徑：

- `float32_to_float16_quiz()`：嚴格照題目 Part 2 程式碼，包含 NaN canonicalize 成 `0x7E00`、single denorm 直接映射為 0。
- `float32_to_float16_full()`：較完整的 integer converter，用於額外探索 NaN payload 與 single denorm。
- `float32_to_float16_ref()`：獨立 reference converter

驗證方式包含：

- 全部 `65536` 個 half precision 位元型態的 roundtrip 檢查
- 特殊值測試：`0`、`-0`、subnormal、normal、`Inf`、`NaN`
- 大量隨機 32-bit 樣本，比對主實作與 reference

這樣可以直接抓出：

- 正負號是否保留
- exponent 進位是否正確
- subnormal 與 min normal 的交界是否正確
- tie case 是否符合 round-to-even

## 延伸問題

1. 哪些輸入會從 normalized 掉進 denormalized 區間？
2. 哪些輸入會因為 round-to-even 從最大 subnormal 進位到最小 normalized？
3. 哪些 NaN payload 會被保留，哪些會被 canonicalize？

## 預期觀察重點

- `0x38800000` 附近是 half min normal 的重要門檻。
- `0x007fffff`、`0x00800000`、`0x00800001` 可以觀察 denormal 與 normal 的切換。
- tie case 會用 LSB 偶數決定往哪邊捨入。
- 題目版本的 `0x7FC00000` 會固定回傳 `0x7E00`，不是保留 payload。
- 題目版本的 single denormalized input 會直接映射為 signed zero。

## 填空答案

- `D01 = 112`
- `D02 = 14`
- `D03 = 12`
