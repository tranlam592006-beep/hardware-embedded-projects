#include <Wire.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>



LiquidCrystal_I2C lcd(0x27, 16, 2);

#define MOTOR1_DIR 6   // HIGH = tien
#define MOTOR1_EN1  A2   // HIGH = chay day xanh
#define MOTOR1_EN2  A3   // HIGH = chay day nâu

#define MOTOR2_EN1 3 
#define MOTOR2_EN2 4
#define MOTOR2_DIR  5  // motor quay thep

#define BT_START 7
#define BT_R_UP  8
#define BT_R_DN  9
#define BT_STOP  10



#define SS1 A6
#define SS2 A7

int R_value = 20;
int R_min = 10;
int R_max = 30;
int R_step = 2;

#define SENSOR_THRESHOLD 500
int lastStateSS2 = LOW;

unsigned long HOME_TIMEOUT = 20000;


// biến đo vị trí
bool needHomeBeforeMove = false;
int lastSetR = 0;
volatile unsigned long countSS2High = 0;
float currentDistance = 0;

// ================== ĐO BÁN KÍNH =============
#define MM_PER_REV    0.2    // 1 vòng cánh quạt motor tịnh tiến đi được 0.0112994 mm
// ================= SETTING =================
#define SENSOR_THRESHOLD 500
 
// Bán kính
float thep = 10.0; // mm     độ rộng thép
float L0 = 10.0;   // mm (đo thực tế)
float D  = 105.0;   // mm (khoảng cách 2 trục)
float targetDistance = 0;

enum State {
  POWER_ON,
  IDLE,
  UON_LAN_CUOI,
  WAIT_MOVE_R_REVERSE,
  WAIT_MOVE_R_FORWARD,
  WAIT_MOVE_R_RIGHT,
  WAIT_MOVE_R_LEFT,
  HOMING,
  HOME_ERROR,
  MOVE_R,
  RUNNING,
  MOVE_R_1_STEP
};

State currentState = IDLE;
State stateBeforeEmergency = IDLE;
unsigned long stateStartTime = 0;
unsigned long lastLCDTime = 0;

bool daKhoiDongMotor1 = false;
bool daKhoiDongMotor2 = false;

// ================= SENSOR =================
int readSS1() {
  int value = analogRead(SS1);
  return (value < SENSOR_THRESHOLD) ? LOW : HIGH;
}

int readSS2() {
  int value = analogRead(SS2);
  return (value < SENSOR_THRESHOLD) ? LOW : HIGH;
}

void countSS234() {
  int currentState = readSS2();

  // Chỉ đếm khi chuyển từ LOW -> HIGH (tránh đếm lặp)
  if (currentState == HIGH && lastStateSS2 == LOW) {
    countSS2High++;
    Serial.print("so1:  ");
    Serial.println(countSS2High);
    delay(100);
  }

  lastStateSS2 = currentState;
}

// điều khiển động cơ
void motor1Forward() {
  digitalWrite(MOTOR1_EN1, LOW);  // dừng
  digitalWrite(MOTOR1_EN2, LOW);  // dừng
  delay(300);
  digitalWrite(MOTOR1_DIR, LOW);  // DOI CHIU
  delay(500);
  digitalWrite(MOTOR1_EN1, HIGH);  // 
  digitalWrite(MOTOR1_EN2, HIGH);  // 
}

void motor1Reverse() {
  digitalWrite(MOTOR1_EN1, LOW);  // dừng
  digitalWrite(MOTOR1_EN2, LOW);  // dừng
  delay(300);
  digitalWrite(MOTOR1_DIR, HIGH);  // lùi
  delay(500);
  digitalWrite(MOTOR1_EN1, HIGH);  // tiến
  digitalWrite(MOTOR1_EN2, HIGH);  // 
}

void motor1Stop() {
  digitalWrite(MOTOR1_EN1, LOW);  // dừng
  digitalWrite(MOTOR1_EN2, LOW);
  daKhoiDongMotor1 = false;

}

void motor2Left(){
  digitalWrite(MOTOR2_EN1, LOW);  // dừng
  digitalWrite(MOTOR2_EN2, LOW);  // dừng
  delay(300);
  digitalWrite(MOTOR2_DIR, LOW);  // tiến
  delay(500);
  digitalWrite(MOTOR2_EN1, HIGH);  // tiến
  digitalWrite(MOTOR2_EN2, HIGH);  // 
}

void motor2Right(){
  digitalWrite(MOTOR2_EN1, LOW);  // dừng
  digitalWrite(MOTOR2_EN2, LOW);  // dừng
  delay(300);
  digitalWrite(MOTOR2_DIR, HIGH);  // tiến
  delay(500);
  digitalWrite(MOTOR2_EN1, HIGH);  // tiến
  digitalWrite(MOTOR2_EN2, HIGH);  // 
}

void motor2Stop() {
  digitalWrite(MOTOR2_EN1, LOW);
  digitalWrite(MOTOR2_EN2, LOW);
  daKhoiDongMotor2 = false;

}

void stopAllMotors() {
  motor1Stop();
  motor2Stop();
}


// ================= STATE =================
void changeState(State newState) {
  currentState = newState;
  stateStartTime = millis();
  lastLCDTime = 0;
  countSS2High = 0;
  lcd.clear();
}

// ================= BUTTON R =================
void readRadiusButtons() {
  if (digitalRead(BT_R_UP) == LOW) {
    delay(80);
    if (digitalRead(BT_R_UP) == LOW) {

      lastSetR = R_value;
      R_value += R_step;

      if (R_value > R_max) {
        R_value = R_max;
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SET R=");
      lcd.print(R_value);
      lcd.print("cm");

      while (digitalRead(BT_R_UP) == LOW);
    }
  }

  if (digitalRead(BT_R_DN) == LOW) {
    delay(80);
    if (digitalRead(BT_R_DN) == LOW) {

      lastSetR = R_value;
      R_value -= R_step;

      if (R_value < R_min) {
        R_value = R_min;
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SET R=");
      lcd.print(R_value);
      lcd.print("mm");

      while (digitalRead(BT_R_DN) == LOW);
    }
  }
}

// ================= LCD =================
void updateLCD(String status) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("R=");
  lcd.print(R_value);
  lcd.print("cm");

  lcd.setCursor(0, 1);
  lcd.print(status);
}

// TÍNH BÁN KÍNH
float calculateTargetDistance(float R_input) {
  float inside = R_input * R_input - D * D;
  return L0 + R_input - sqrt(inside);      
}

// ================= PULSE COUNT =================
void countSS2() {
  int currentState = readSS2();
  // Chỉ đếm khi chuyển từ LOW -> HIGH (tránh đếm lặp)
  if (currentState == HIGH && lastStateSS2 == LOW) {
    countSS2High++;
    Serial.print("Count SS2: ");
    Serial.println(countSS2High);
    // float rev = countSS2High;
    // currentDistance = rev * MM_PER_REV;
  }
  lastStateSS2 = currentState;
}

// ================ BANG GIA TRI =================
int BanKinh[] = {5, 4, 3, 3, 2, 2, 2};
int soVongUon[] = {197, 157, 244, 75, 281, 182,102};
int soLanUon = 0;
int demSoLanUon = 0;
unsigned long soVongUonCuoi = 0;

bool khoiTaoRunning = false;
bool doiChieu = true;
bool dangUon = false;


void setup() {
  pinMode(MOTOR1_DIR, OUTPUT);
  pinMode(MOTOR1_EN1, OUTPUT);
  pinMode(MOTOR1_EN2, OUTPUT);

  pinMode(MOTOR2_DIR, OUTPUT);
  pinMode(MOTOR2_EN1, OUTPUT);
  pinMode(MOTOR2_EN2, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);

  pinMode(BT_START, INPUT_PULLUP);
  pinMode(BT_R_UP, INPUT_PULLUP);
  pinMode(BT_R_DN, INPUT_PULLUP);
  pinMode(BT_STOP, INPUT_PULLUP);

  pinMode(SS1, INPUT);
  pinMode(SS2, INPUT);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("MAY UON THEP");
  delay(2000);
  lcd.clear();
  changeState(POWER_ON);

  // countSS2();
}

void loop() { 
  // if (readSS1() == LOW) 
  // {
  //   motor1Stop();

  //   countSS2High = 0;
  //   currentDistance = 0;
  //   // pinMode(A4, OUTPUT);
  //   // pinMode(A5, OUTPUT);
  //   for(uint8_t i = 0; i<6; i++)
  //   {
  //     // digitalWrite(A4,1);
  //     // digitalWrite(A5,1);
  //     digitalWrite(13,1);
  //     delay(100);
  //     // digitalWrite(A4,0);
  //     // digitalWrite(A5,0);
  //     digitalWrite(13,0);
  //     delay(100);
  //   }
  //   // lcd.init();
  //   lcd.setCursor(0, 0);
  //   lcd.print("OK           ");
  //   lcd.setCursor(0, 1);
  //   lcd.print("CHINH BAN KINH     ");
  //   // delay(1000);
  //   for(uint16_t i = 0; i<10000; i++)
  //   {
  //     digitalWrite(13,1);
  //     delay(100);
  //     digitalWrite(13,0);
  //     delay(100);
  //   }

  switch (currentState) {
    case POWER_ON:

      if (millis() - lastLCDTime > 300) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GIU START");
        lcd.setCursor(0, 1);
        lcd.print("DE KHOI DONG");
        lastLCDTime = millis();
      }

      // Nhan START de bat dau
      if (digitalRead(BT_START) == LOW) {
        delay(50);
        if (digitalRead(BT_START) == LOW) {
          changeState(WAIT_MOVE_R_REVERSE);
          while (digitalRead(BT_START) == LOW);
        }
      }
      break;


    case HOMING:
      if (readSS1() == LOW) {
        motor1Stop();
        khoiTaoRunning = false;
        countSS2High = 0;
        currentDistance = 0;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("OK           ");
        lcd.setCursor(0, 1);
        lcd.print("CHINH BAN KINH     ");
        delay(2000);
        // while(1) 
        // {
        //   digitalWrite(13,1);
        //   delay(100);
        //   digitalWrite(13,0);
        //   delay(100);
        // }
        changeState(IDLE);
      }
      if (millis() - stateStartTime > 75000) {
        motor1Stop();
        changeState(HOME_ERROR);
      }
      break;


    case HOME_ERROR:
      lcd.setCursor(0, 0);
      lcd.print("LOI              ");

      lcd.setCursor(0, 1);
      lcd.print("START -> RETRY      ");

      // Nhấn START để home lại
      if (digitalRead(BT_START) == LOW) {
        delay(50);
        if (digitalRead(BT_START) == LOW) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("QUAY LAI...");
          delay(500);
          changeState(WAIT_MOVE_R_REVERSE);
          while (digitalRead(BT_START) == LOW);
        }
      }
      break;
    
    case MOVE_R_1_STEP:
      if (daKhoiDongMotor1 == false) {
          motor1Forward();
          daKhoiDongMotor1 = true;
          demSoLanUon++;
      }
      if (millis() - stateStartTime >= 5000) {
        countSS2();
        // Điều kiện dừng
        if (countSS2High >= 335) {
          motor1Stop();
          doiChieu = !doiChieu;
          if (dangUon == false) {
            dangUon = true;
            if(doiChieu){
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("UON SANG TRAI");
              delay(1000);
              changeState(WAIT_MOVE_R_LEFT);
            } else {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("UON SANG PHAI");
              delay(1000);
              changeState(WAIT_MOVE_R_RIGHT);
            }
          }
        }
      }
      break;

    case WAIT_MOVE_R_FORWARD:
      // Chỉ chạy 1 lần khi mới vào state
      if (daKhoiDongMotor1 == false) {
          motor1Forward();
          daKhoiDongMotor1 = true;
      }
      // Sau 5 giây thì chuyển state
      if (millis() - stateStartTime >= 5000) {
          changeState(MOVE_R);
      }
      break;

    case WAIT_MOVE_R_REVERSE:
      // Chỉ chạy 1 lần khi mới vào state
      if (daKhoiDongMotor1 == false) {
          motor1Reverse();
          daKhoiDongMotor1 = true;
      }
      // Sau 5 giây thì chuyển state
      if (millis() - stateStartTime >= 5000) {
          changeState(HOMING);
      }
      break;

    case WAIT_MOVE_R_RIGHT:
      // Chỉ chạy 1 lần khi mới vào state
      if (daKhoiDongMotor2 == false) {
          motor2Right();
          daKhoiDongMotor2 = true;
      }
      // Sau 5 giây thì chuyển state
      if (millis() - stateStartTime >= 5000) {
          changeState(RUNNING);
      }
      break;

    case WAIT_MOVE_R_LEFT:
      // Chỉ chạy 1 lần khi mới vào state
      if (daKhoiDongMotor2 == false) {
          motor2Left();
          daKhoiDongMotor2 = true;
      }
      // Sau 5 giây thì chuyển state
      if (millis() - stateStartTime >= 5000) {
          changeState(RUNNING);
      }
      break;

    case UON_LAN_CUOI:
      if (daKhoiDongMotor1 == false) {
        motor1Forward();
        demSoLanUon++;
        daKhoiDongMotor1 = true;
      }
      if (millis() - stateStartTime >= 5000) {
        countSS2();
        // Điều kiện dừng
        if (countSS2High >= soVongUonCuoi) {
          motor1Stop();
          doiChieu = !doiChieu;
          if (dangUon == false) {
            dangUon = true;
            if(doiChieu){
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("UON SANG TRAI");
              delay(1000);
              changeState(WAIT_MOVE_R_LEFT);
            } else {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("UON SANG PHAI");
              delay(1000);
              changeState(WAIT_MOVE_R_RIGHT);
            }
          }
        }
      }
      break;

    case MOVE_R:
      countSS2();
      // Serial.print("Count SS2: ");
      // Serial.println(countSS2High);
      // Điều kiện dừng
      if (countSS2High >= 800) {
        motor1Stop();
        lcd.setCursor(0, 0);
        lcd.print("DA DEN VI TRI");
        delay(2000);

        lcd.setCursor(0, 1);
        lcd.print("START DE UON");

        // Cho nhan START
        if (digitalRead(BT_START) == LOW) {
          delay(50);
          if (digitalRead(BT_START) == LOW) {

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("CHUAN BI UON");
            delay(1000);
            changeState(RUNNING);
            while (digitalRead(BT_START) == LOW);
          }
        }
      }
      break;

    case RUNNING:
      // Lan dau vao RUNNING
      if (khoiTaoRunning == false) {
        int index = (R_value - R_min) / R_step;
        soLanUon = BanKinh[index];
        soVongUonCuoi = soVongUon[index];
        khoiTaoRunning = true;
        demSoLanUon = 0;
        doiChieu = true;
        dangUon = false;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("so lan uon ");
        lcd.print(soLanUon + 1);
        delay(2000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("START = UON");
        lcd.setCursor(0, 1);
        lcd.print("STOP = DUNG");
        delay(2000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("LAN UON THU ");
        lcd.print(demSoLanUon + 1);
        delay(2000);
        changeState(MOVE_R_1_STEP);
        }

        // STOP = dung lan uon hien tai
        if (digitalRead(BT_STOP) == LOW) {
          delay(50);
          if (digitalRead(BT_STOP) == LOW) {
            motor2Stop();
            dangUon = false;
            // demSoLanUon++;
            delay(1000);
            while (digitalRead(BT_STOP) == LOW);

            // Neu da du so lan uon chinh
            if (demSoLanUon == soLanUon) {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("UON LAN CUOI");
              delay(1000);
              changeState(UON_LAN_CUOI);
              break;
            }

            if (demSoLanUon > soLanUon ) {
                // khoiTaoRunning = false;
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("HOAN THANH");
                delay(3000);
                changeState(WAIT_MOVE_R_REVERSE); // về home
              break;
            }
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("START UON TIEP");
          }
        }

        // START 
        if (digitalRead(BT_START) == LOW) {
            delay(50);
            if (digitalRead(BT_START) == LOW) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("LAN UON THU ");
            lcd.print(demSoLanUon + 1);
            delay(2000);
            //demSoLanUon++;
            changeState(MOVE_R_1_STEP);
            while (digitalRead(BT_START) == LOW);
          }
        }
      break;

    case IDLE:
      readRadiusButtons();

      if (millis() - lastLCDTime > 300) { 
        updateLCD("READY"); 
        lastLCDTime = millis(); 
      }

      if (digitalRead(BT_START) == LOW) {
            // countSS2High = 0;
            // currentDistance = 0;
            demSoLanUon = 0;
            changeState(WAIT_MOVE_R_FORWARD);
      }
      break;
  }
}
