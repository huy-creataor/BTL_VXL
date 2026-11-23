# CO3009 - Hệ thống đèn giao thông 2 tuyến đường (XNUCLEO-F103RB)

Bài tập lớn môn **Vi Xử Lý – Vi Điều Khiển (CO3009)** — Khoa Khoa học và Kỹ thuật Máy tính, Trường Đại học Bách Khoa - ĐHQG TP.HCM.

## 1. Mô tả dự án

Hệ thống điều khiển đèn giao thông ngã tư 2 tuyến đường chạy trên mạch **XNUCLEO-F103RB** (vi điều khiển `STM32F103RB`), sử dụng bộ lập lịch **O(1) Scheduler** (hỗ trợ one-shot task) và kiến trúc tách lớp phần cứng / phần mềm:

- **Chế độ tự động (`AUTO`)**: Hai tuyến đèn Đỏ – Xanh – Vàng hoạt động luân phiên theo thời gian cài đặt, không bao giờ xảy ra trạng thái hai tuyến cùng Xanh.
- **Chế độ bằng tay (`MANUAL`)**: Người dùng chuyển trạng thái đèn bằng nút nhấn an toàn.
- **Chế độ chỉnh thời gian (`TUNING`)**: Thay đổi thời gian Đỏ – Xanh – Vàng bằng nút nhấn, kiểm tra ràng buộc hợp lệ (`T_RED == T_GREEN + T_AMBER`), hiển thị trực tiếp trên LCD.
- **Hiển thị môi trường**: Đọc cảm biến `DHT20` qua `I2C1` (non-blocking) và hiển thị nhiệt độ, độ ẩm lên `LCD` I2C.

## 2. Cấu trúc thư mục

```text
CO3009_TrafficLight_F103RB/
├── .gitignore
├── README.md
├── docs/
│   ├── PINOUT_AND_IOC_CONFIG.md   # Bảng chân GPIO thống nhất, cấu hình .ioc & bảng kiểm thử Bước 3
│   └── WORK_DIVISION.md           # Phân công 2 thành viên (Code + Báo cáo), nhánh Git & lộ trình 5 tuần
└── Core/
    ├── Inc/
    │   ├── pin_config.h           # Ánh xạ chân GPIO, địa chỉ I2C, extern HAL handles (chỉ driver .c mới include)
    │   ├── global.h               # Trạng thái hệ thống (FSM), thời gian cài đặt, biến dùng chung
    │   ├── scheduler.h            # Bộ lập lịch O(1) (SCH_Update O(1), hỗ trợ one-shot task)
    │   ├── software_timer.h       # Bộ đếm thời gian phần mềm (tick 10 ms)
    │   ├── button.h               # Driver đọc nút nhấn + chống dội + nhấn giữ
    │   ├── traffic_light.h        # Driver điều khiển 2 cụm đèn giao thông
    │   ├── dht20.h                # Driver cảm biến nhiệt độ - độ ẩm DHT20 (I2C, non-blocking)
    │   ├── lcd.h                  # Driver màn hình LCD I2C 16x2
    │   ├── fsm_automatic.h        # Máy trạng thái chế độ tự động
    │   ├── fsm_manual.h           # Máy trạng thái chế độ bằng tay
    │   └── fsm_tuning.h           # Máy trạng thái chế độ chỉnh thời gian
    └── Src/                       # Mã nguồn .c tương ứng (sẽ được hiện thực theo phân công)
```

> **Quy tắc kiến trúc bắt buộc**: Không đặt lệnh HAL (`HAL_GPIO_*`, `HAL_I2C_*`, `HAL_Delay`) bên trong các file `fsm_*.c` hay `scheduler.c`. Chỉ các driver phần cứng (`button.c`, `traffic_light.c`, `dht20.c`, `lcd.c`) và `main.c` mới được include `pin_config.h` / `main.h`.

## 3. Bảng phân công tóm tắt (Nhóm 2 thành viên)

Xem chi tiết tại [docs/WORK_DIVISION.md](docs/WORK_DIVISION.md).

| Thành viên | Phần mã nguồn phụ trách | Phần báo cáo phụ trách | Nhánh Git chính |
|---|---|---|---|
| **Thành viên A** | Khởi tạo CubeIDE (`.ioc`), Bước 3.1–3.3, `scheduler.c`, `software_timer.c`, `button.c`, `traffic_light.c`, `global.c`, `fsm_automatic.c`, `fsm_manual.c` | Kiến trúc phân lớp, bảng chân GPIO & `.ioc` (Mục 3.1), kiểm thử LED/đèn/nút, thiết kế & phân tích `O(1)` Scheduler, module timer/button/đèn, sơ đồ & an toàn FSM Auto/Manual | `feature/scheduler`, `feature/gpio-drivers`, `feature/fsm-auto-manual` |
| **Thành viên B** | Bước 3.4 (I2C scan), `dht20.c`, `lcd.c`, `fsm_tuning.c`, tích hợp `main.c` | Giới thiệu đề tài, bus I2C1 & kết quả scan (Mục 3.4), module DHT20 (non-blocking), module LCD & giao diện menu, FSM Tuning + sơ đồ chuyển chế độ tổng thể + ràng buộc thời gian, kiểm thử tổng thể, tổng hợp PDF & tag nộp bài | `feature/i2c-dht20-lcd`, `feature/fsm-tuning`, `feature/integration` |

## 4. Bảng thống nhất chân GPIO & ngoại vi (`XNUCLEO-F103RB`)

Xem chi tiết cấu hình `.ioc` và công thức tính `TIM2` tại [docs/PINOUT_AND_IOC_CONFIG.md](docs/PINOUT_AND_IOC_CONFIG.md).

| Thiết bị | Chân Arduino / Khe | Chân STM32 | Chế độ `.ioc` | User Label |
|---|---|---|---|---|
| Đèn giao thông 1 (`A`) | `D3` | `PB3` | Output Push-Pull | `TL1_A` |
| Đèn giao thông 1 (`B`) | `D4` | `PB5` | Output Push-Pull | `TL1_B` |
| Đèn giao thông 2 (`A`) | `D5` | `PB4` | Output Push-Pull | `TL2_A` |
| Đèn giao thông 2 (`B`) | `D6` | `PB10` | Output Push-Pull | `TL2_B` |
| Nút nhấn 1 (Mode) | `D7` | `PA8` | Input | `BTN_1` |
| Nút nhấn 2 (Inc / Phase) | `D8` | `PA9` | Input | `BTN_2` |
| Nút nhấn 3 (Dec / Warn) | `D9` | `PC7` | Input | `BTN_3` |
| Nút nhấn 4 (Set / Confirm) | `D10` | `PB6` | Input | `BTN_4` |
| Cảm biến DHT20 & LCD | Khe `I2C` đỏ (`D15`/`D14`) | `PB8` (`SCL`) / `PB9` (`SDA`) | `I2C1` (Remap) | — |
| LED có sẵn 1 / 2 | Onboard | `PA5` / `PC9` | Output Push-Pull | `LED_USER1` / `LED_USER2` |
| Nút USER có sẵn | Onboard | `PC13` | Input | `BTN_USER` |
| Nhịp Scheduler (10 ms) | Nội bộ | `TIM2` + NVIC | Internal Clock | `htim2` |

## 5. Hướng dẫn tạo dự án STM32CubeIDE, Build và Nạp

1. **Tạo dự án đúng chip `STM32F103RB` (tránh lỗi chỉ nạp được 1 lần)**:
   - Mở **STM32CubeIDE** → *File → New → STM32 Project*.
   - Chọn tab **Board Selector**, tìm và chọn **`NUCLEO-F103RB`**.
   - Đặt tên dự án trùng hoặc copy thư mục `Core/Inc/*.h` từ repo này vào dự án.
2. **Bắt buộc bật Serial Wire Debug (giải phóng `PB3`/`PB4` và cho phép nạp nhiều lần)**:
   - Mở file `.ioc` → **Pinout & Configuration → System Core → SYS → Debug = `Serial Wire`**.
3. **Cấu hình chân & ngoại vi trong `.ioc`**:
   - Thiết lập đầy đủ chân GPIO và User Label theo bảng mục 4.
   - Bật **I2C1** (`I2C`) và kiểm tra chân gán là **`PB8` (`I2C1_SCL`)** và **`PB9` (`I2C1_SDA`)** (vì `PB6` đã dùng cho `BTN_4`).
   - Bật **TIM2** (`Clock Source = Internal Clock`), bật ngắt **TIM2 global interrupt** trong **NVIC**, tính `PSC` và `ARR` theo `f_TIM` thực tế để đạt chu kỳ ngắt **10 ms**.
   - Lưu `.ioc` và Generate Code.
4. **Build và Nạp**:
   - Kết nối mạch `XNUCLEO-F103RB` qua cáp USB (ST-Link).
   - Nhấn **Build** (`Ctrl+B`), đảm bảo 0 error.
   - Nhấn **Run** hoặc **Debug** (`F11`) để nạp chương trình xuống board.

## 6. Quy trình làm việc với Git

- Nhánh `main`: lưu bản ổn định và gắn tag nộp bài (`v1.0-final`).
- Nhánh `dev`: nhánh tích hợp chung của cả nhóm.
- Mỗi tính năng tạo nhánh riêng từ `dev` (`git checkout -b feature/<ten-chuc-nang>`), commit bằng tài khoản GitHub cá nhân với message rõ ràng (ví dụ: `scheduler: add one-shot task support`).

### Đẩy repo local lên GitHub lần đầu

```bash
git remote add origin https://github.com/<tai-khoan>/<ten-repo>.git
git push -u origin main
git push -u origin dev
```
