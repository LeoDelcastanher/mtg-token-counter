# 3461BS-1 7-Segment Display — Verified Pinout

## Type: Common Anode
- Digit pins: HIGH = digit ON
- Segment pins: LOW = segment ON

## Pin Mapping

| Display Pin | Function        | Arduino Pin |
|-------------|-----------------|-------------|
| 1           | Digit 1 (left)  | 9           |
| 2           | Segment A (top) | 2           |
| 3           | Segment F (top-left) | 3      |
| 4           | Digit 2         | 10          |
| 5           | Digit 3         | 11          |
| 6           | Segment B (top-right) | 4     |
| 7           | Segment E (bottom-left) | 5   |
| 8           | Segment D (bottom) | 6        |
| 9           | Segment DP (dot) | 7          |
| 10          | Segment C (bottom-right) | 8  |
| 11          | Segment G (middle) | 13       |
| 12          | Digit 4 (right) | 12          |

## Segment Array Order
`int seg[] = {2, 3, 4, 5, 6, 8, 13}; // A, F, B, E, D, C, G`

## Verified Digit Patterns (common anode: 0=on, 1=off)
```cpp
byte digits[10][7] = {
  {0,0,0,0,0,0,1}, // 0
  {1,1,0,1,1,0,1}, // 1
  {0,1,0,0,0,1,0}, // 2
  {0,1,0,1,0,0,0}, // 3
  {1,0,0,1,1,0,0}, // 4
  {0,0,1,1,0,0,0}, // 5
  {0,0,1,0,0,0,0}, // 6
  {0,1,0,1,1,0,1}, // 7
  {0,0,0,0,0,0,0}, // 8
  {0,0,0,1,1,0,0}, // 9
};
```
