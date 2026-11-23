# Phân công công việc nhóm 2 thành viên (Code + Báo cáo)

Nguyên tắc chung: **Ai lập trình phần nào sẽ viết báo cáo cho phần đó**, sử dụng chung các giao diện hàm (`.h`) đã thống nhất trong `Core/Inc/` để hai thành viên có thể phát triển song song mà không xung đột.

---

## 1. Bảng phân chia tổng thể

| Hạng mục | Thành viên A (Core Engine, GPIO Drivers, FSM Auto & Manual) | Thành viên B (I2C Drivers, FSM Tuning, Tích hợp hệ thống) |
|---|---|---|
| **Vai trò chính** | Kiến trúc nền tảng, thời gian thực & điều khiển đèn/nút cơ bản | Giao tiếp I2C, cảm biến, giao diện LCD, chỉnh thời gian & tích hợp |
| **Kiểm thử phần cứng (Bước 3)** | Bước 3.1 (nháy LED `PA5`), Bước 3.2 (bảng tổ hợp đèn `TL1`, `TL2`), Bước 3.3 (mức logic 4 nút + `PC13`) | Bước 3.4 (I2C1 scan trên `PB8`/`PB9`, xác định địa chỉ `DHT20` và `LCD`) |
| **File `.c` phụ trách** | `Core/Src/scheduler.c`<br>`Core/Src/software_timer.c`<br>`Core/Src/button.c`<br>`Core/Src/traffic_light.c`<br>`Core/Src/global.c`<br>`Core/Src/fsm_automatic.c`<br>`Core/Src/fsm_manual.c` | `Core/Src/dht20.c`<br>`Core/Src/lcd.c`<br>`Core/Src/fsm_tuning.c`<br>`Core/Src/main.c` *(đăng ký task, tích hợp & kiểm thử cùng A)* |
| **File `.h` liên quan** | `pin_config.h`, `global.h`, `scheduler.h`, `software_timer.h`, `button.h`, `traffic_light.h`, `fsm_automatic.h`, `fsm_manual.h` | `pin_config.h`, `global.h`, `dht20.h`, `lcd.h`, `fsm_tuning.h` |
| **Nhánh Git** | `feature/cube-init`<br>`feature/scheduler`<br>`feature/gpio-drivers`<br>`feature/fsm-auto-manual` | `feature/i2c-dht20-lcd`<br>`feature/fsm-tuning`<br>`feature/integration` |
| **Phần Báo cáo phụ trách** | 1. Kiến trúc phân lớp phần mềm/phần cứng.<br>2. Bảng tra chân GPIO & cấu hình `.ioc` (Mục 3.1).<br>3. Kết quả kiểm thử LED, tổ hợp đèn 2-bit, nút nhấn.<br>4. Thiết kế & phân tích `O(1)` Scheduler (chọn Array/Linked List, giải thích `SCH_Update` O(1), hỗ trợ one-shot task, bằng chứng kiểm thử).<br>5. Hiện thực `software_timer`, `button` (chống dội, nhấn giữ), `traffic_light`.<br>6. Sơ đồ & phân tích **FSM Automatic** và **FSM Manual** (chứng minh không bao giờ 2 tuyến cùng Xanh). | 1. Giới thiệu đề tài, yêu cầu & phân công nhóm.<br>2. Cấu hình `I2C1` (`PB8`/`PB9`) & kết quả I2C scan (Mục 3.4).<br>3. Module `DHT20` (giao thức datasheet, công thức nhiệt độ/độ ẩm, kiểm tra CRC, cơ chế non-blocking không `HAL_Delay`).<br>4. Module `LCD` (driver I2C, thiết kế giao diện hiển thị / menu cho cả 3 chế độ).<br>5. Sơ đồ **FSM Tuning** + **Sơ đồ chuyển đổi giữa 3 chế độ lớn** (trạng thái đèn khi chuyển, reset timer/task, hiển thị LCD, ràng buộc hợp lệ `T_RED = T_GREEN + T_AMBER` & xử lý giá trị sai).<br>6. Kiểm thử tổng thể, xử lý tình huống biên, hoàn thiện `README.md`, xuất PDF `CO3009_MSSV1_MSSV2_Report.pdf` & gắn tag `v1.0-final`. |

---

## 2. Phân tích khối lượng & điểm giao tiếp giữa A và B

### Điểm đồng bộ bắt buộc (Contract thông qua file `.h`)
1. **`scheduler.h`**: Thành viên A hoàn thành `SCH_Init`, `SCH_Add_Task`, `SCH_Delete_Task`, `SCH_Update`, `SCH_Dispatch_Tasks` trước khi cả hai chạy module phụ trợ ở Tuần 3.
2. **`global.h`**: Chứa biến `status`, `time_red_s`, `time_green_s`, `time_amber_s`, `countdown_lane1`, `countdown_lane2`, `env_temperature`, `env_humidity`.
   - `fsm_automatic.c` (A) đọc `time_red_s`, `time_green_s`, `time_amber_s` và cập nhật `countdown_lane1`, `countdown_lane2`.
   - `fsm_tuning.c` (B) chỉnh `temp_time_*`, kiểm tra hợp lệ rồi mới ghi vào `time_red_s`, `time_green_s`, `time_amber_s`.
   - `dht20.c` (B) ghi `env_temperature`, `env_humidity`, `env_data_valid`; `lcd.c` (B) đọc các biến trên để hiển thị.
3. **`button.h`**: Quy ước chỉ số nút thống nhất cho cả 3 FSM:
   - `BTN_MODE` (`0` - `PA8`): chuyển chế độ (`AUTO` → `MANUAL` → `TUNING` → `AUTO`, hoặc từ `TUNING` thoát an toàn).
   - `BTN_INC` (`1` - `PA9`): tăng +1s (Tuning) / chuyển pha Đỏ1-Xanh2 ↔ Xanh1-Đỏ2 (Manual).
   - `BTN_DEC` (`2` - `PC7`): giảm -1s (Tuning) / chuyển trạng thái an toàn `MAN_ALL_RED` hoặc Vàng nhấp nháy (Manual).
   - `BTN_SET` (`3` - `PB6`): xác nhận & chuyển mục Đỏ → Xanh → Vàng (Tuning) / quay về `AUTO` (Manual).
4. **Quy tắc an toàn khi chuyển chế độ (cả 2 cùng kiểm tra và giải thích khi Demo)**:
   - Khi rời `AUTO` sang `MANUAL` hoặc `TUNING`: xóa cờ timer cũ (`clearTimer`), đảm bảo trạng thái chuyển tiếp luôn đi qua pha an toàn (không bao giờ giữ hai tuyến cùng Xanh).
   - Khi từ `TUNING` hoặc `MANUAL` quay lại `AUTO`: gọi `fsm_automatic_init()` để reset bộ đếm thời gian và đặt cặp đèn ban đầu an toàn (`AUTO_RED1_GREEN2`).
   - Khi đang đèn Vàng mà nhấn `BTN_MODE`: xử lý dứt điểm hoặc đưa ngay về trạng thái khởi tạo an toàn của chế độ đích.

---

## 3. Lộ trình 5 tuần (theo mục B0 trong hướng dẫn)

| Tuần | Công việc Thành viên A | Công việc Thành viên B | Mốc kiểm tra chung (Merge vào `dev`) |
|---|---|---|---|
| **Tuần 1** | Tạo project CubeIDE (`.ioc`), test LED `PA5`, test bảng tổ hợp 2 đèn (`TL1`, `TL2`), xác định mức logic 4 nút + `PC13`. | Tạo/cập nhật GitHub repo & collaborator, kiểm tra I2C scan trên `PB8/PB9` tìm địa chỉ `DHT20` và `LCD`. | Cập nhật xong `pin_config.h` & bảng Bước 3 trong `PINOUT_AND_IOC_CONFIG.md`. |
| **Tuần 2** | Hiện thực & test `scheduler.c` (`O(1)` `SCH_Update`, `SCH_Delete_Task`, one-shot task, test 2 LED 0.5s & 1s). | Chuẩn bị driver nền `lcd.c` (I2C PCF8574) và đọc datasheet `DHT20`, test độc lập trên khung tối giản. | Merge `feature/scheduler` vào `dev`. |
| **Tuần 3** | Hiện thực `software_timer.c`, `button.c`, `traffic_light.c`, `global.c` và demo test trên scheduler. | Hoàn thiện `dht20.c` (non-blocking) & `lcd.c` chạy task định kỳ trên scheduler (mỗi 2s đọc DHT20 in ra LCD). | Merge `feature/gpio-drivers` và `feature/i2c-dht20-lcd` vào `dev`. |
| **Tuần 4** | Vẽ & hiện thực `fsm_automatic.c` và `fsm_manual.c`; viết nháp mục báo cáo phần A. | Vẽ & hiện thực `fsm_tuning.c` (ràng buộc `T_RED = T_GREEN + T_AMBER`), tích hợp 3 FSM + LCD menu trong `main.c`; viết nháp mục báo cáo phần B. | Hệ thống chạy đủ 3 chế độ trên nhánh `dev`. |
| **Tuần 5** | Test tình huống biên (nhấn liên tục, chuyển chế độ lúc đèn Vàng), rà soát không còn `HAL_*` trong `fsm_*.c`. | Test tổng thể chạy liên tục, ghép & định dạng báo cáo PDF `CO3009_MSSV1_MSSV2_Report.pdf`, cập nhật `README.md`, merge `dev` → `main` & tag `v1.0-final`. | Nộp báo cáo PDF + link GitHub trước hạn `04/12/2026`. |

---

## 4. Quy tắc Git & Commit đánh giá cá nhân

1. Mỗi thành viên **bắt buộc commit bằng tài khoản Git cá nhân** (`git config user.name`, `git config user.email` đúng của mình) vì giảng viên chấm đóng góp qua lịch sử commit.
2. Không commit trực tiếp lên `main`. Làm việc trên nhánh `feature/*`, kiểm tra build sạch (0 error, 0 warning nghiêm trọng) rồi merge vào `dev`.
3. Định dạng commit message: `<module>: <mô tả ngắn gọn>` (ví dụ: `scheduler: implement O(1) SCH_Update and relative delay insertion`, `dht20: add non-blocking measurement state machine`).
