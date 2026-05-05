# Quiz5 Problem C: unsigned_high_prod 與寬乘法 fallback

這個 lab 用來對照 quiz5 Problem C 的幾個重點：

- `unsigned_high_prod`
- `signed_high_prod` 加修正項
- `mul_u64_u32_shr` 類型的 64x32 -> shifted result fallback
- `size_mul` / overflow sentinel `SIZE_MAX`

## 核心想法

### 1. `unsigned_high_prod`

題目本身是 32-bit `unsigned`。本 lab 先實作題目版本：

```c
return (unsigned)signed_hi + (sx & y) + (sy & x);
```

其中第二個修正項的 `sy & __C01__` 對應 `sy & x`。程式會直接驗證：

```c
unsigned_high_prod32(0xFFFFFFFF, 0xFFFFFFFF) == 0xFFFFFFFE
```

接著再提供同一公式的 64-bit 類比版本，方便和 `__int128` oracle 做大量測試。

對 64-bit `x` 與 `y`，可用 signed 版本的高位乘積再補修正項：

```c
unsigned_high_prod(x, y) =
    signed_high_prod((int64_t)x, (int64_t)y)
    + (sign_mask_x & y)
    + (sign_mask_y & x)
```

其中 `sign_mask_x` 與 `sign_mask_y` 是全 0 或全 1 的遮罩，取決於 `x`、`y` 的符號位。

本 lab 會用 `__int128` 當 oracle，先測固定案例，再跑大量 pseudo-random cases，確認公式正確。

### 2. `mul_u64_u32_shr` fallback

Linux 的 `math64.h` 常見寫法是把 64-bit 與 32-bit 的乘積拆成兩段：

- `p0 = lo32(a) * b`
- `p1 = hi32(a) * b`
- `carry = p0 >> 32`
- `mid = p1 + carry`

這樣就能得到一個 96-bit 的中間結果，再依 shift 做右移。

這個 lab 會把 partial products 印出來，讓你看到 carry 是怎麼往上傳的。

### 3. `size_mul` overflow sentinel

`size_mul` 這類 helper 的典型做法是：

- 沒 overflow -> 回傳正常乘積
- 有 overflow -> 回傳 `SIZE_MAX`

這是 Linux 常見的 sentinel 模式。

## 填空答案對照

| Blank | Value |
| --- | --- |
| C01 | `x` |
| C02 | `FFFFFFFE` |

## 如何執行

在此目錄下：

```bash
make test
```

如果只想編譯：

```bash
make
```

清除 build 結果：

```bash
make clean
```

## 預期觀察重點

- `unsigned_high_prod` 的公式應該和 `__int128` oracle 完全一致。
- `mul_u64_u32_shr` 的 demo 會列出 `p0`、`p1`、`carry`、`mid`，顯示 carry propagation。
- `size_mul_like` 遇到溢位時會回傳 `SIZE_MAX`。

## 檔案

- [`src/c_mul_high_lab.c`](./src/c_mul_high_lab.c)
- [`Makefile`](./Makefile)
