# Bảng chân GPIO thống nhất & Cấu hình STM32CubeIDE (`.ioc`)

Tài liệu này là nguồn chuẩn duy nhất để cả 2 thành viên cấu hình file `.ioc` trên `STM32CubeIDE` và điền vào **Mục 3.1** của báo cáo.

---

## 1. Bảng tra chân GPIO (Dùng cho Mục 3.1 Báo cáo)

Đã đối chiếu từ schematic `XNUCLEO-F103RB` (Hình 1 & Hình 2 trong tài liệu hướng dẫn BTL CO3009):

| Thiết bị | Chân Arduino | Chân STM32 | Chế độ GPIO | User Label đặt trong `.ioc` | Ghi chú kỹ thuật |
|---|---|---|---|---|---|
| Module đèn giao thông 1 (`A`) | `D3` | `PB3` | Output Push-Pull | `TL1_A` | Cần `SYS -> Debug = Serial Wire` để giải phóng `PB3` (`JTDO`) |
| Module đèn giao thông 1 (`B`) | `D4` | `PB5` | Output Push-Pull | `TL1_B` | Kết hợp với `TL1_A` tạo 4 trạng thái đèn tuyến 1 |
| Module đèn giao thông 2 (`A`) | `D5` | `PB4` | Output Push-Pull | `TL2_A` | Cần `SYS -> Debug = Serial Wire` để giải phóng `PB4` (`NJTRST`) |
| Module đèn giao thông 2 (`B`) | `D6` | `PB10` | Output Push-Pull | `TL2_B` | Kết hợp với `TL2_A` tạo 4 trạng thái đèn tuyến 2 |
| Module nút nhấn 1 (Nút 0 - Mode) | `D7` | `PA8` | Input | `BTN_1` | `BTN_MODE` (chuyển chế độ `AUTO` / `MANUAL` / `TUNING`) |
| Module nút nhấn 1 (Nút 1 - Inc) | `D8` | `PA9` | Input | `BTN_2` | `BTN_INC` (đổi pha ở Manual / tăng +1s ở Tuning) |
| Module nút nhấn 2 (Nút 2 - Dec) | `D9` | `PC7` | Input | `BTN_3` | `BTN_DEC` (cảnh báo/pha phụ ở Manual / giảm -1s ở Tuning) |
| Module nút nhấn 2 (Nút 3 - Set) | `D10` | `PB6` | Input | `BTN_4` | `BTN_SET` (xác nhận lưu ở Tuning / về Auto) |
| Cảm biến DHT20 & LCD (`SCL`) | `D15` (Khe I2C đỏ) | `PB8` | `I2C1_SCL` | — | **Bắt buộc remap I2C1 sang `PB8/PB9`** |
| Cảm biến DHT20 & LCD (`SDA`) | `D14` (Khe I2C đỏ) | `PB9` | `I2C1_SDA` | — | Không dùng `PB6/PB7` vì `PB6` đã nối `D10` (`BTN_4`) |
| LED có sẵn 1 | `D13` / Onboard | `PA5` | Output Push-Pull | `LED_USER1` | Dùng cho kiểm thử nạp chương trình & nhịp 500 ms |
| LED có sẵn 2 | Onboard | `PC9` | Output Push-Pull | `LED_USER2` | Dùng cho kiểm thử scheduler 1000 ms |
| Nút nhấn có sẵn (USER) | Onboard | `PC13` | Input | `BTN_USER` | `BTN_USER_IDX` (tùy chọn reset thời gian mặc định) |

> **Cảnh báo xung đột chân `PB6` vs `I2C1`**: Mặc định CubeMX gán `I2C1` vào `PB6`/`PB7`, nhưng trên mạch `XNUCLEO-F103RB`, chân Arduino `D10` nối trực tiếp vào `PB6` và khe I2C đỏ nối vào `D15/D14` (`PB8/PB9`). Do đó **bắt buộc** click vào `PB8` chọn `I2C1_SCL` và `PB9` chọn `I2C1_SDA` trong `.ioc`.

---

## 2. Checklist cấu hình `.ioc` từng bước

1. **Tạo dự án**: Chọn tab **Board Selector** → tìm **`NUCLEO-F103RB`** (chip `STM32F103RBTx`).
2. **SYS Debug (Bắt buộc)**: *System Core → SYS → Debug = `Serial Wire`*.
   - Tránh khóa SWD khiến mạch chỉ nạp được 1 lần.
   - Giải phóng `PB3` và `PB4` khỏi chức năng JTAG để dùng làm GPIO điều khiển đèn `TL1_A`, `TL2_A`.
3. **GPIO**:
   - Đặt `PB3` (`TL1_A`), `PB5` (`TL1_B`), `PB4` (`TL2_A`), `PB10` (`TL2_B`), `PA5` (`LED_USER1`), `PC9` (`LED_USER2`) là `GPIO_Output` (Push-Pull, Low speed).
   - Đặt `PA8` (`BTN_1`), `PA9` (`BTN_2`), `PC7` (`BTN_3`), `PB6` (`BTN_4`), `PC13` (`BTN_USER`) là `GPIO_Input` (kiểm tra Pull-up/No-pull tùy mạch nút Grove tại Bước 3.3).
4. **I2C1**:
   - *Connectivity → I2C1 → Mode = `I2C`*, tốc độ `Standard Mode (100 kHz)`.
   - Chuyển chân `I2C1_SCL` sang `PB8` và `I2C1_SDA` sang `PB9`.
5. **TIM2 (Scheduler Tick = 10 ms)**:
   - *Timers → TIM2 → Clock Source = `Internal Clock`*.
   - Tab *NVIC Settings*: tích chọn **TIM2 global interrupt**.
   - Tính `PSC` và `ARR` theo công thức:
     $$T = \frac{(PSC + 1) \times (ARR + 1)}{f_{\text{TIM}}} = 0.01\text{ s}\ (10\text{ ms})$$

| Tần số `f_TIM2` (trong Clock Configuration) | `PSC` (Prescaler) | `ARR` (Counter Period) | Chu kỳ ngắt |
|---|---|---|---|
| `8 MHz` (HSI mặc định) | `7999` | `9` | `10 ms` |
| `64 MHz` (HSI + PLL mặc định của Board Nucleo-F103RB) | `6399` | `99` | `10 ms` |
| `72 MHz` (HSE + PLL) | `7199` | `99` | `10 ms` |

6. **Trong `main.c`**: Nhớ gọi `HAL_TIM_Base_Start_IT(&htim2);` trước vòng `while(1)` và định nghĩa `HAL_TIM_PeriodElapsedCallback()` gọi `SCH_Update()`.

---

## 3. Bảng kết quả kiểm thử phần cứng (Đã xác nhận trên mạch thật)

### 3.1 Bảng tổ hợp tín hiệu đèn (Mục 3.2 - Thành viên A thực hiện)

Đã xác nhận trực tiếp trên mạch đèn HCMUT (in trên PCB: `00-OFF, 01-YEL, 10-GRE, 11-RED`) và cập nhật 8 macro `TL_SIG_*` trong [Core/Inc/pin_config.h](../Core/Inc/pin_config.h):

| Tín hiệu A (`TLx_A`) | Tín hiệu B (`TLx_B`) | Trạng thái đèn Tuyến 1 | Trạng thái đèn Tuyến 2 |
|---|---|---|---|
| `0` | `0` | `OFF` (Tắt) | `OFF` (Tắt) |
| `0` | `1` | `AMBER` (Vàng) | `AMBER` (Vàng) |
| `1` | `0` | `GREEN` (Xanh) | `GREEN` (Xanh) |
| `1` | `1` | `RED` (Đỏ) | `RED` (Đỏ) |

### 3.2 Mức logic nút nhấn (Mục 3.3 - Thành viên A thực hiện)

- Mức logic khi **nhấn** `BTN_1`..`BTN_4` (Grove, dùng `GPIO_PULLUP` nội): `GPIO_PIN_RESET` (mức thấp) → đã cấu hình `BTN_PRESSED_STATE = GPIO_PIN_RESET` trong [Core/Inc/pin_config.h](../Core/Inc/pin_config.h).
- Mức logic khi **nhấn** `BTN_USER` (`PC13` trên board): `GPIO_PIN_RESET` (mức thấp).

### 3.3 Địa chỉ I2C 7-bit (Mục 3.4 - Thành viên B thực hiện)

- Địa chỉ `DHT20` tìm được: `0x38` (7-bit) → truyền cho HAL là `0x38 << 1 = 0x70`.
- Địa chỉ `LCD` PCF8574 tìm được trên mạch thực tế: `0x21` (7-bit) → truyền cho HAL là `0x21 << 1 = 0x42`, đã cập nhật `LCD_I2C_ADDR_7BIT = 0x21U` trong [Core/Inc/pin_config.h](../Core/Inc/pin_config.h) kèm tự dò địa chỉ trong `lcd.c`.
