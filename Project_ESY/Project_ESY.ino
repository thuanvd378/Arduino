#include <Wire.h>
#include <RTClib.h>
#include <TM1637Display.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <Servo.h>
//Khởi tạo cảnh báo (biến đếm 5 lần)
int sus = 0;

//Servo SG90
Servo lock;
int lockPin = 8;

// Nút và đèn
enum SysMode { NORMAL, WAIT_ENROLL, WAIT_DELETE };
SysMode sysMode = NORMAL;
bool lastStable;
int button = 2;
int ledgreen = 3;
int ledred = 4;
int buzzer  = 5;
bool nextIsEnroll = true;   // true = ENROLL, false = DELETE
bool waitRelease  = false;  // khóa cho tới khi nhả

// Module TM1637
#define CLK A0
#define DIO A1
TM1637Display display(CLK, DIO);

// Module RTC
#define SDA A4
#define SCL A5
RTC_DS3231 rtc;
const int TZ_OFFSET_HOURS = 7;
unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 1000;

//Module AS608 Fingerprint
uint8_t goodImageStreak = 0;
SoftwareSerial fingerSerial(6, 7);  // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);
//Function Fingerprint
void handleButton();
void enrollFinger();
void deleteFinger();
int  nextFreeID();
void searchFinger();

// Chương trình chính
void setup() {
  sus = 0;
  lock.attach(lockPin);
  lock.write(0); 
  Serial.begin(115200);   // chọn baudrate của PC
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("RTC error!");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println(F("Sensor detected and verified."));
  } else {
    Serial.println(F("ERROR: Could not find a valid AS608 sensor."));
    Serial.println(F("Check wiring & power, then reset."));
    while (true) { delay(1); }
  }

  finger.getParameters();
  Serial.print(F("Sensor capacity: ")); Serial.println(finger.capacity);

  Serial.println(F("\n*** Ready. Place a finger. ***\n"));

  pinMode(button, INPUT);
  lastStable = digitalRead(button);
  pinMode(ledgreen, OUTPUT);
  pinMode(ledred, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(ledred, LOW);
  digitalWrite(ledgreen, LOW);
  display.clear();
  display.setBrightness(7);
  Serial.println("READY");   
  lockDoor();
}
// Setup thời gian thực
void realTime() {
  DateTime now = rtc.now();
  int hhmm = now.hour() * 100 + now.minute();
  display.showNumberDecEx(hhmm, 0b01000000, true, 4, 0);
}

void sendCurrentTime(int id) {
  DateTime now = rtc.now();
  char buf[6];
  sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  Serial.print(id);
  Serial.print(',');
  Serial.println(buf);
}
void refreshTimer() {
  unsigned long nowMs = millis();
  if (nowMs - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = nowMs;
    realTime();
  }
}
// Kết nối tới chương trình C#
String cmdBuf;
void pollSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      cmdBuf.trim();
      if (cmdBuf == "GETDATA") {
        Serial.println("END_OF_DATA");
      }
      cmdBuf = "";  // reset buffer
    } else {
      cmdBuf += c;
      if (cmdBuf.length() > 32) cmdBuf.remove(0); // avoid overflow
    }
  }
}
// Xử lí nút ấn
void handleButton() {
  bool reading = digitalRead(button);

  /* Giữ nút trong 5ms */
  if (reading != lastStable) {
    delay(5);
    reading = digitalRead(button);
  }

  /* -------- khoá nút cho tới khi nhả hoàn toàn -------- */
  if (waitRelease) {
    if (reading == LOW) {            
      waitRelease = false;           
    }
    lastStable = reading;
    return;                          
  }

  /* -------- phát hiện cạnh lên (LOW → HIGH = nhấn) ----- */
  if (reading == HIGH && lastStable == LOW) {
  if (sysMode == NORMAL) {                 // lần bấm đầu → ENROLL
    sysMode = WAIT_ENROLL;
    digitalWrite(ledgreen, HIGH);
    digitalWrite(ledred,   HIGH);
    Serial.println(F("ENTER ENROLL: place finger or press again to DELETE"));
    waitRelease = true;                    // buộc phải nhả nút

  } else if (sysMode == WAIT_ENROLL) {     // đang ở ENROLL
    /* ------ luôn chuyển sang DELETE, bất kể có ngón tay hay không ------ */
    sysMode = WAIT_DELETE;                 // 
    goodImageStreak = 0;                   // xoá chuỗi đếm ảnh (nếu có)
    Serial.println(F("SWITCH to DELETE mode"));
    waitRelease = true;                    // đợi nhả nút
  }
  }

  lastStable = reading;
}
void processSpecialMode() {
  switch (sysMode) {

    /* ---------- chờ đặt tay để ENROLL ---------- */
    case WAIT_ENROLL: {
  uint8_t p = finger.getImage();

  if (p == FINGERPRINT_OK) {
    /* kiểm tra ảnh có thật sự chuyển đổi được không */
    if (finger.image2Tz(1) == FINGERPRINT_OK) {
      goodImageStreak++;
    } else {
      goodImageStreak = 0;                 // ảnh nhiễu, reset đếm
    }
  } else if (p == FINGERPRINT_NOFINGER) {
    goodImageStreak = 0;                   // không có ngón tay
  }

  /* chỉ khi liên tiếp 2 lần OK mới bắt đầu enrollFinger() */
  if (goodImageStreak >= 2) {
    goodImageStreak = 0;                   // reset cho lần sau
    enrollFinger();                        // hàm cũ, blocking
    digitalWrite(ledgreen, LOW);
    digitalWrite(ledred,   LOW);
    sysMode = NORMAL;
  }
  break;
}

    /* ---------- chờ đặt tay để DELETE ---------- */
    case WAIT_DELETE:
      if (finger.getImage() == FINGERPRINT_OK) {   // đợi ngón tay
        deleteFinger();                            // dùng hàm cũ (xóa đúng vân tay)
        digitalWrite(ledgreen, LOW);
        digitalWrite(ledred,   LOW);
        sysMode = NORMAL;
      }
      break;

    default: break;
  }
}
//--------------------------------
void loop() {
  if (sus >= 5) {
    emergency();
  } else {
    pollSerial();
    handleButton();          // xử lý nút
    processSpecialMode();    // xử lý ENROLL / DELETE chờ
    if (sysMode == NORMAL) { // chỉ tìm vân tay khi ở chế độ bình thường
      searchFinger();
    }
    refreshTimer();
  }
}
//--------------------------------

//LOCK OR UNLOCK
// ---------- mở khoá ----------
void unlockDoor() {
  lock.attach(lockPin);     // gắn lại PWM
  lock.write(180);          // xoay mở
  delay(500);               // chờ servo xoay hết
  lock.detach();            // ngừng PWM, servo sẽ đứng im
}

// ---------- khoá ----------
void lockDoor() {
  lock.attach(lockPin);
  lock.write(0);            // xoay khoá
  delay(500);
  lock.detach();
}

//Xử lí âm thanh nút ấn
void btn_success() {
   tone(buzzer, 1318); // Nốt E6
   digitalWrite(ledred, LOW);
  digitalWrite(ledgreen, HIGH);
  delay(150);
  digitalWrite(ledgreen, LOW);
  noTone(buzzer);
  delay(50);
  tone(buzzer, 1568); // Nốt G6
  digitalWrite(ledgreen, HIGH);
  delay(150);
  digitalWrite(ledgreen, LOW);
  noTone(buzzer);
}
void btn_fail() {
  tone(buzzer, 110); // Nốt A2
  digitalWrite(ledgreen, LOW);
  digitalWrite(ledred, HIGH);
  delay(250);
  digitalWrite(ledred, LOW);
  noTone(buzzer);
  tone(buzzer, 98);  // Nốt G2
  digitalWrite(ledred, HIGH);
  delay(250);
  noTone(buzzer);
  digitalWrite(ledred, LOW);
}
void emergency() {
    digitalWrite(ledred, HIGH);
    for (int freq = 500; freq <= 1500; freq += 50) {
      tone(buzzer, freq);
      delay(10); 
    }
    delay(500);
    digitalWrite(ledred, LOW);
    for (int freq = 1500; freq >= 500; freq -= 50) {
      tone(buzzer, freq);
      delay(10);
    }
    delay(500);
    digitalWrite(ledred, HIGH);
}
//Xử lí module vân tay

// ----------------------------------------------------
// Đăng ký vân tay mới
// ----------------------------------------------------
void enrollFinger() {
  int id = nextFreeID();
  if (id < 0) {
    btn_fail();
    Serial.println(F("Database full – cannot enroll more fingers."));
    return;
  }
  Serial.print(F("Enrolling at ID #")); Serial.println(id);

  // Bước 1 : Chụp ảnh vân tay lần 1
  Serial.println(F("Place finger…"));
  while (finger.getImage() != FINGERPRINT_OK) { /* wait */ }
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    btn_fail();
    Serial.println(F("Image conversion failed."));
    return;
  }

  // Bước 2 : Chụp ảnh vân tay lần 2
  Serial.println(F("Remove finger."));
  delay(1000);
  Serial.println(F("Place same finger again…"));
  while (finger.getImage() != FINGERPRINT_OK) { /* wait */ }
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    btn_fail();
    Serial.println(F("Image conversion failed (2)."));
    return;
  }

  // Bước 3 : Tạo model và lưu trữ vân tay
  if (finger.createModel() != FINGERPRINT_OK) {
    btn_fail();
    Serial.println(F("Could not create fingerprint model."));
    return;
  }
  if (finger.storeModel(id) == FINGERPRINT_OK) {
    btn_success();
    Serial.println(F("Enrolled successfully!"));
  } else {
    btn_success();
    Serial.println(F("ERROR: Could not store fingerprint."));
  }
}

// ----------------------------------------------------
// Xóa vân tay đã đăng ký
// ----------------------------------------------------
void deleteFinger() {
  // Chụp lại vân tay
  if (finger.getImage() != FINGERPRINT_OK) {
    btn_fail();
    Serial.println(F("No finger detected."));
    return;
  }
  if (finger.image2Tz() != FINGERPRINT_OK) {
    btn_fail();
    Serial.println(F("Image conversion failed."));
    return;
  }

  // Tìm kiếm trong database
  if (finger.fingerFastSearch() != FINGERPRINT_OK) {
    btn_fail();
    Serial.println(F("Finger not found in database – nothing deleted."));
    return;
  }

  uint16_t id = finger.fingerID;
  Serial.print(F("Found ID #")); Serial.print(id);
  Serial.println(F(" – deleting…"));

  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    btn_success();
    Serial.println(F("Deleted successfully."));
  } else {
    btn_fail();
    Serial.println(F("ERROR: Could not delete fingerprint."));
  }
}

// ----------------------------------------------------
// Tìm slot ID còn trống cho vân tay. Trả lại -1 nếu đã hết slot
// ----------------------------------------------------
int nextFreeID() {
  if (finger.getTemplateCount() != FINGERPRINT_OK) return -1;
  for (uint16_t id = 1; id <= finger.capacity; id++) {
    if (finger.loadModel(id) != FINGERPRINT_OK) {
      return id;
    }
  }
  return -1;   // none free
}

// ----------------------------------------------------
// Tìm kiếm vân tay đã có trong Database
// ----------------------------------------------------
void searchFinger() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return;  // no finger

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println(F("Failed to convert image"));
    btn_fail();
    return;
  }

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    sus = 0;
    btn_success();
    unlockDoor();
    Serial.print(F("Match!  ID #"));
    Serial.print(finger.fingerID);
    Serial.print(F("  Confidence: "));
    Serial.println(finger.confidence);
    sendCurrentTime(finger.fingerID);
    delay(100);
    lockDoor();
  } else if (p == FINGERPRINT_NOTFOUND) {
    sus++;
    btn_fail();
    Serial.println(F("No match found."));
  } else {
    btn_fail();
    Serial.print(F("Search error, code: "));
    Serial.println(p);
  }

  delay(1000);
}