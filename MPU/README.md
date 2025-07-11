# Bộ Ổn Định 3 Trục Cho Servo Dùng MPU‑6050 (Arduino)

> **Ngắn gọn:** Dự án này hiện thực một bộ ổn định 3 trục (yaw–pitch–roll) sử dụng cảm biến gia tốc/ con quay MPU‑6050, lọc Kalman & bộ lọc thông thấp để điều khiển 3 servo. Mục đích là giữ thăng bằng hoặc định hướng thiết bị (camera, nền tảng nhỏ, v.v.) một cách mượt mà.

## Tính năng chính

- Đọc dữ liệu DMP của **MPU‑6050** (quaternion, YPR) qua I²C ở 400 kHz.
- **Kalman filter** trên 3 trục giúp giảm nhiễu tốc độ quay.
- **Low‑pass filter** (α = 0,03) làm mượt thêm tín hiệu góc.
- Ánh xạ góc (‑90 → 90 °) sang xung servo (0 → 180 °).
- Xuất dữ liệu qua **Serial Plotter** (baud 38 400) để quan sát.
- Tự hiệu chuẩn yaw trong 300 mẫu đầu.

## Phần cứng cần có

| Linh kiện                             | Số lượng | Ghi chú                |
| ------------------------------------- | -------- | ---------------------- |
| Arduino (Uno / Nano / Pro Mini …)     | 1        | 5 V logic              |
| Cảm biến MPU‑6050                     | 1        | GY‑521 phổ biến        |
| Servo 9 g                             | 3        | Hoặc loại khác 0–180 ° |
| Nguồn 5 V ≥ 2 A                       | 1        | Tách riêng cho servo   |
| Tụ 4 700 µF                           | ≥1       | Giảm sụt áp trên V+    |
| Dây AWG16 (nguồn) + jumper (tín hiệu) | —        |                        |

> **Lưu ý nguồn:** Servo lấy dòng lớn; không cấp thẳng từ cổng 5 V của Arduino.

## Sơ đồ nối dây (tham khảo)

```
MPU‑6050      Arduino
VCC  ── 5 V
GND  ── GND
SCL  ── A5 (SCL)
SDA  ── A4 (SDA)
INT  ── D2  (INTERRUPT_PIN)

Servo Yaw   PWM D10
Servo Pitch PWM D9
Servo Roll  PWM D8

Nguồn 5 V servo ⟷ V+ PCA / tụ 4700 µF ⟷ GND chung
```

## Phụ thuộc phần mềm

```text
Arduino IDE ≥ 1.8.19  (hoặc VS Code + PlatformIO)
I2Cdevlib/MPU6050       (kèm trong repo hoặc qua Library Manager)
Kalman                  (jdtackett/Kalman)
Servo                   (mặc định của Arduino)
```

## Cài đặt & nạp chương trình

1. Clone repo:
   ```bash
   git clone https://github.com/<user>/<repo>.git
   ```
2. Mở `MPU6050_Servo_Stabilizer.ino` trong Arduino IDE.
3. Cài thư viện còn thiếu (Sketch → Include Library → Manage Libraries).
4. Chọn bo mạch & cổng COM, nhấn **Upload**.

## Cách sử dụng

1. Cấp nguồn cho servo **trước**, tránh khởi động hụt áp.
2. Mở **Serial Plotter** (38 400 baud) để quan sát YPR lọc.
3. Đặt hệ thống trên mặt phẳng cố định \~ 5 s để tự cân bằng yaw.
4. Khi di chuyển cảm biến, 3 servo sẽ bù lại góc.

## Tuỳ chỉnh

| Tham số     | Mô tả                        | Vị trí               |
| ----------- | ---------------------------- | -------------------- |
| `alpha`     | Hệ số lọc thông thấp (0 – 1) | `lowPassFilter*.cpp` |
| `dt`        | Chu kỳ tính Kalman (s)       | đầu file             |
| Offset gyro | `setXGyroOffset()` …         | hàm `setup()`        |

## Cấu trúc thư mục

```
├── src/
│   └── MPU6050_Servo_Stabilizer.ino
├── docs/
│   └── wiring.pdf (sơ đồ chi tiết)
└── README.md
```

## Giấy phép

Mã nguồn phát hành theo giấy phép **MIT**. Xem `LICENSE` để biết chi tiết.

## Đóng góp

PR và issue luôn được hoan nghênh! Vui lòng fork repo, tạo nhánh mới và gửi pull request.

---

© 2025 Tên tác giả. Điều chỉnh theo nhu cầu dự án của bạn.

