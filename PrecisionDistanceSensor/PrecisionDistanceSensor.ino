#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* Địa chỉ I2C của LCD */
#define LCD_ADDR 0x27         

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

const int trigPin = 8;
const int echoPin = 7;

const uint8_t  N_SAMPLES   = 5;          // số mẫu dùng để lấy median
const float    US_TO_CM    = 0.01724f;   // 1 µs ≈ 0.01724 cm
const float    MIN_CM      = 2.0f;       // giới hạn gần nhất tin cậy
const float    MAX_CM      = 400.0f;     // giới hạn xa nhất tin cậy

/* ───────────────────────────── */
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(9600);          // bật/tắt tùy ý
  lcd.init();                  // khởi động LCD
  lcd.begin(16, 2);            // cần cho một số fork LiquidCrystal_I2C
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Khoang cach:");
}

/* Đo một lần – trả về cm; <0 nếu timeout */
float measureOneCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long us = pulseIn(echoPin, HIGH, 25000);   // 25 ms ≈ 4 m
  if (us == 0) return -1.0f;
  return us * US_TO_CM;
}

/* ───────────────────────────── */
void loop() {
  float buf[N_SAMPLES];

  /* 1️⃣  Lấy N_SAMPLES mẫu */
  for (uint8_t i = 0; i < N_SAMPLES; ++i) {
    buf[i] = measureOneCM();
    delay(50);
  }

  /* 2️⃣  Lấy median */
  for (uint8_t i = 0; i < N_SAMPLES - 1; ++i)
    for (uint8_t j = i + 1; j < N_SAMPLES; ++j)
      if (buf[j] < buf[i]) { float t = buf[i]; buf[i] = buf[j]; buf[j] = t; }

  float dist = buf[N_SAMPLES / 2];

  /* 3️⃣  Lọc giá trị không hợp lệ, ép về 0 cm */
  if (dist < MIN_CM || dist > MAX_CM) dist = 0.0f;

  /* 4️⃣  Hiển thị lên LCD */
  lcd.setCursor(0, 1);
  char txt[8];
  dtostrf(dist, 5, 1, txt);    // " xx.x"
  lcd.print(txt);
  lcd.print(" cm   ");         // đệm khoảng trắng xoá ký tự thừa

  /* 5️⃣  Ghi ra Serial (tùy chọn) */
  Serial.print(dist, 1);
  Serial.println(" cm");

  delay(200);
}
