# Quiz5 Problem B: UB / aliasing

這個 lab 對照 quiz5 Problem B 的幾個重點：

- `int_size_is_w`
- signed left shift 的 UB
- `BIT(n)` 的 `unsigned long` 寫法
- `GENMASK`
- strict aliasing

## 目標

本 lab 會同時展示三件事：

1. signed left shift 在 `1 << (sizeof(int) * CHAR_BIT - 1)` 這類情況下會觸發 UB。
2. `BIT(n)` 應該改成 `1UL << n`，避免 signed overflow / signed shift 問題。
3. `int *` 與 `float *` 的 pointer cast 會踩 strict aliasing；`memcpy` 是安全寫法，`union` 是常見實務寫法。

## 如何執行

在此目錄下：

```bash
make test
```

會編譯 O0 / O2 / UBSan 版本、產生 assembly，並執行範例程式。

若只想產生 assembly：

```bash
make asm
```

## 預期觀察重點

- `signed_shift_bad()` 會在 UBSan 下報出 shift UB。
- `BIT_UL()` 會用 `unsigned long` 正確產生 bit mask。
- `alias_violation()` 在 `-O0` 與 `-O2` 的行為可能不同，這正是 strict aliasing 的重點。
- `memcpy` 與 `union` 版本會保留預期 bit pattern。

## 延伸問題對照

| 延伸題 | 參考答案 | 說明 |
|---|---:|---|
| B01 | `wrapped` | 對應位移 / 位元運算觀察到的包裝結果 |
| B02 | `undefined behavior` | 對應 signed shift 與 strict aliasing 類型的 UB |

## 檔案

- [`src/ub_alias_lab.c`](./src/ub_alias_lab.c)
- [`Makefile`](./Makefile)
