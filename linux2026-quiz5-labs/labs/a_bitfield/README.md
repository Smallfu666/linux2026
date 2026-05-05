# Quiz5 Problem A: bitfield / replace_nbytes

這個 lab 用來對照 quiz5 Problem A 的幾個重點：

- `replace_nbytes`
- `FIELD_PREP`
- `GENMASK`
- `1u << 32` 的 undefined behavior

## 目標

本 lab 會用一個 32-bit 的範例，把任意連續 byte field 替換成指定值，並驗證：

1. `n = 1` 時，只會替換最低 1 byte。
2. `n = 4` 時，整個 32-bit word 會被完整替換。
3. 題目案例 `replace_nbytes(0xAABBCCDD, 0, 3, 0x112233)` 會得到 `0xAA112233`。
4. 自我檢查案例 `replace_nbytes(0x12345678, 1, 2, 0xABCD)` 會得到 `0x12ABCD78`。
5. 若直接寫 `1u << 32`，在 32-bit `unsigned int` 上是 UB，不能這樣做。
6. `GENMASK` / `FIELD_PREP` 的寫法可以安全地構造 mask 與欄位值。

## 如何執行

在此目錄下：

```bash
make test
```

或只執行：

```bash
make run
```

## 預期觀察重點

- `n=1` 的結果應該只改到最低 1 byte。
- `n=4` 的結果應該變成 `0xAA112233`。
- `A01` 的實際值是 `mask << shift`，用來清掉原本欄位。
- `A02` 的實際值是 `mask`，用來裁切輸入 `y`。
- `popcount(mask)` 會等於 `8 * n`。
- `safe mask` 會用條件分支避免 `1u << 32`。

## 延伸問題對照

| 延伸題 | 參考答案 | 說明 |
|---|---:|---|
| A01 | `mask << shift` | 對應 `FIELD_PREP` 類型的位移組裝 |
| A02 | `mask` | 對應用來遮罩的欄位 mask |
| A03 | `undefined behavior` | 對應 `1u << 32` 這類超出位元寬度的位移 |
| A04 | `AA112233` | `n=4` 時的完整替換結果 |

## 檔案

- [`src/bitfield_lab.c`](./src/bitfield_lab.c)
- [`Makefile`](./Makefile)
