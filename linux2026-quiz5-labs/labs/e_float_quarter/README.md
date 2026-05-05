# Problem E: float_quarter

這個實驗用純整數位元操作實作 `float_quarter`，重點放在：

- exponent 由 `0 / 1 / 2 / 3` 進入不同分支時的行為
- NaN / Inf 的保留規則
- subnormal 與 min normal 的邊界
- round-to-even 是否正確處理 sticky bit

## 執行方式

在此目錄內：

```sh
make test
```

如果想看小 demo，可以：

```sh
make demo
```

## 測試設計

程式內含兩條路徑：

- `float_quarter()`：主要實作
- `float_quarter_ref()`：獨立 reference converter

驗證方式包含：

- 特殊值測試：`0`、`-0`、`subnormal`、`normal`、`Inf`、`NaN`
- `exp=0/1/2/3` 的邊界樣本
- 大量隨機 32-bit 樣本，比對主實作與 reference
- `float_eighth` 小 demo，方便觀察連續縮小兩次後的位元變化

## 延伸問題

1. 哪些 `exp` 會直接減 2 而不需要重排 fraction？
2. 哪些 `exp` 會掉進 subnormal，必須做 shift 與 round-to-even？
3. 哪些輸入會因為 tie case 而進位到最小 normalized？

## 預期觀察重點

- `exp=3` 時，quarter 之後仍是 normalized。
- `exp=2` 和 `exp=1` 會進入 subnormal 路徑。
- `0x00800000` quarter 之後會得到 `0x00200000`，是很好的對照點。
- `0x00000003` quarter 之後會落在 round-to-even 的臨界。

## 填空答案

- `E01 = 2`
- `E02 = 1`
- `E03 = 00200000`
