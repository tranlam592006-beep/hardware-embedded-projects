// ================= BLYNK =================
#define BLYNK_TEMPLATE_ID "TMPL61dgUWUu1"
#define BLYNK_TEMPLATE_NAME "NhaThongMinh"
#define BLYNK_AUTH_TOKEN "QaXwHkWcUwE9HYcOn2bLakD2BumQUCo1"

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Adafruit_Fingerprint.h>

char ssid[] = "Loi";
char pass[] = "12345678";

BlynkTimer timer;

// ================= ADMIN PHONE =================
String ADMIN_PHONE = "+84346441022";   // Sửa thành số điện thoại admin

// ================= GPIO =================
#define MQ2_SENSOR1 34
#define MQ2_SENSOR2 39
#define MQ2_SENSOR3 36

#define DHT11_SENSOR1 25
#define DHT11_SENSOR2 33
#define DHT11_SENSOR3 32
#define DHTTYPE DHT11

#define LED1 14
#define LED2 27
#define LED3 26

#define BUZZER 23

#define I2C_SDA 21
#define I2C_SCL 22

#define FINGER_RX 19
#define FINGER_TX 18

#define SIM_RX_PIN 16   // ESP32 RX2 <- SIM TX
#define SIM_TX_PIN 17   // ESP32 TX2 -> SIM RX

#define DOOR 5
#define FAN 4

#define BUTTON_ADD 15
#define BUTTON_DELETE 13

// ================= BLYNK VIRTUAL PIN =================
#define VP_TEMP1 V0
#define VP_HUM1  V1
#define VP_TEMP2 V2
#define VP_HUM2  V3
#define VP_TEMP3 V4
#define VP_HUM3  V5

#define VP_LED1  V10
#define VP_LED2  V11
#define VP_LED3  V12
#define VP_FAN   V13
#define VP_DOOR  V14

#define VP_STATUS V30

// ================= MODULE =================
DHT dht1(DHT11_SENSOR1, DHTTYPE);
DHT dht2(DHT11_SENSOR2, DHTTYPE);
DHT dht3(DHT11_SENSOR3, DHTTYPE);

const unsigned long MQ2_WARMUP_TIME = 60000; // 60 giay
LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo doorServo;

HardwareSerial FingerSerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&FingerSerial);

HardwareSerial SimSerial(2);

// ================= BIẾN =================
float t1 = 0, h1 = 0, t2 = 0, h2 = 0, t3 = 0, h3 = 0;
int gas1 = 0, gas2 = 0, gas3 = 0;

bool fireMode = false;
bool doorOpenedByFinger = false;
bool smsFireSent = false;

unsigned long doorOpenTime = 0;
unsigned long buzzerOffTime = 0;
bool buzzerTimerActive = false;

const int GAS_THRESHOLD = 3900;     // chỉnh theo thực tế
const int DOOR_CLOSE_ANGLE = 90;
const int DOOR_OPEN_ANGLE  = 180;

// ================= SIM SMS =================
void sendSMS(String message)
{
    Serial.println("Dang gui SMS...");
    SimSerial.println("AT");
    delay(500);

    SimSerial.println("AT+CMGF=1");
    delay(500);

    SimSerial.print("AT+CMGS=\"");
    SimSerial.print(ADMIN_PHONE);
    SimSerial.println("\"");
    delay(500);

    SimSerial.print(message);
    delay(500);

    SimSerial.write(26);   // Ctrl+Z
    delay(5000);

    Serial.println("Da gui lenh SMS");
}

// ================= THIẾT BỊ =================
void openDoor()
{
    doorServo.write(DOOR_OPEN_ANGLE);
    Blynk.virtualWrite(VP_DOOR, 1);
}

void closeDoor()
{
    doorServo.write(DOOR_CLOSE_ANGLE);
    Blynk.virtualWrite(VP_DOOR, 0);
}

void buzzerOn()
{
    digitalWrite(BUZZER, HIGH);
}

void buzzerOff()
{
    digitalWrite(BUZZER, LOW);
}

void buzzerOnFor5s()
{
    buzzerOn();
    buzzerOffTime = millis() + 2000;
    buzzerTimerActive = true;
}

void handleBuzzerTimer()
{
    if (buzzerTimerActive && millis() >= buzzerOffTime)
    {
        buzzerOff();
        buzzerTimerActive = false;
    }
}

void fanOn()
{
    digitalWrite(FAN, HIGH);
    Blynk.virtualWrite(VP_FAN, 1);
}

void fanOff()
{
    digitalWrite(FAN, LOW);
    Blynk.virtualWrite(VP_FAN, 0);
}

// ================= BLYNK CONTROL =================
BLYNK_WRITE(VP_LED1)
{
    digitalWrite(LED1, param.asInt());
}

BLYNK_WRITE(VP_LED2)
{
    digitalWrite(LED2, param.asInt());
}

BLYNK_WRITE(VP_LED3)
{
    digitalWrite(LED3, param.asInt());
}

BLYNK_WRITE(VP_FAN)
{
    if (!fireMode)
        digitalWrite(FAN, param.asInt());
}

BLYNK_WRITE(VP_DOOR)
{
    if (!fireMode)
    {
        if (param.asInt()) openDoor();
        else closeDoor();
    }
}

// ================= BUTTON =================
bool isButtonPressed(int pin)
{
    if (digitalRead(pin) == LOW)
    {
        delay(50);
        if (digitalRead(pin) == LOW)
        {
            while (digitalRead(pin) == LOW)
            {
                Blynk.run();
            }
            delay(50);
            return true;
        }
    }
    return false;
}

// ================= DHT =================
void readDHT()
{
    float nt1 = dht1.readTemperature();
    float nh1 = dht1.readHumidity();

    float nt2 = dht2.readTemperature();
    float nh2 = dht2.readHumidity();

    float nt3 = dht3.readTemperature();
    float nh3 = dht3.readHumidity();

    if (!isnan(nt1)) t1 = nt1;
    if (!isnan(nh1)) h1 = nh1;
    if (!isnan(nt2)) t2 = nt2;
    if (!isnan(nh2)) h2 = nh2;
    if (!isnan(nt3)) t3 = nt3;
    if (!isnan(nh3)) h3 = nh3;

    Blynk.virtualWrite(VP_TEMP1, t1);
    Blynk.virtualWrite(VP_HUM1, h1);
    Blynk.virtualWrite(VP_TEMP2, t2);
    Blynk.virtualWrite(VP_HUM2, h2);
    Blynk.virtualWrite(VP_TEMP3, t3);
    Blynk.virtualWrite(VP_HUM3, h3);
}

// ================= LCD =================
void updateLCD()
{
    lcd.clear();

    // Nếu đang báo cháy thì ưu tiên hiển thị cảnh báo
    if (fireMode)
    {
        lcd.setCursor(0, 0);
        lcd.print("!!! CANH BAO !!!");

        lcd.setCursor(0, 1);
        lcd.print("NGUY HIEM");
        return;
    }

    // Dòng 1: 3 nhiệt độ
    lcd.setCursor(0, 0);
    lcd.print((int)t1);
    lcd.print("C ");
    lcd.print((int)t2);
    lcd.print("C ");
    lcd.print((int)t3);
    lcd.print("C ");

    // Dòng 2: 3 độ ẩm
    lcd.setCursor(0, 1);
    lcd.print((int)h1);
    lcd.print("% ");
    lcd.print((int)h2);
    lcd.print("% ");
    lcd.print((int)h3);
    lcd.print("% ");
}

// ================= GAS =================
void readGas()
{
    if (millis() < MQ2_WARMUP_TIME)
    {
        Serial.println("MQ2 dang lam nong, bo qua du lieu...");
        return;
    }

    gas1 = analogRead(MQ2_SENSOR1);
    gas2 = analogRead(MQ2_SENSOR2);
    gas3 = analogRead(MQ2_SENSOR3);

    if (gas1 > GAS_THRESHOLD || gas2 > GAS_THRESHOLD || gas3 > GAS_THRESHOLD)
    {
        if (!fireMode)
        {
            fireMode = true;
            smsFireSent = false;

            buzzerOn();
            fanOn();
            openDoor();

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("CANH BAO GAS!");
            lcd.setCursor(0, 1);
            lcd.print("MO CUA + QUAT");

            Blynk.virtualWrite(VP_STATUS, "CANH BAO GAS - DA MO CUA");

            if (!smsFireSent)
            {
                sendSMS("CANH BAO: Phat hien khi gas/chay! He thong da mo cua va bat quat.");
                smsFireSent = true;
            }
        }
    }
    else
    {
        if (fireMode)
        {
            fireMode = false;
            smsFireSent = false;

            buzzerOff();
            fanOff();
            closeDoor();

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("GAS BINH THUONG");
            lcd.setCursor(0, 1);
            lcd.print("DA DONG CUA");

            Blynk.virtualWrite(VP_STATUS, "Gas binh thuong");
        }
    }
}

// ================= VÂN TAY: NHẬN DIỆN =================
void checkFingerprint()
{
    if (fireMode) return;

    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) return;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) return;

    p = finger.fingerFastSearch();

    if (p == FINGERPRINT_OK)
    {
        Serial.print("Dung van tay ID: ");
        Serial.println(finger.fingerID);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("VAN TAY DUNG");
        lcd.setCursor(0, 1);
        lcd.print("DANG MO CUA");

        Blynk.virtualWrite(VP_STATUS, "Van tay dung - da mo cua");

        openDoor();
        doorOpenedByFinger = true;
        doorOpenTime = millis();
    }
    else
    {
        Serial.println("Sai van tay!");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("LOI VAN TAY!");
        lcd.setCursor(0, 1);
        lcd.print("CANH BAO ADMIN");

        Blynk.virtualWrite(VP_STATUS, "Sai van tay");

        buzzerOnFor5s();
        sendSMS("CANH BAO: Co nguoi quet sai van tay/mo cua sai!");
    }
}

// ================= VÂN TAY: THÊM =================
uint8_t enrollFingerprint(uint16_t id)
{
    int p = -1;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DAT TAY LAN 1");
    Serial.println("Dat ngon tay lan 1...");

    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        Blynk.run();
    }

    p = finger.image2Tz(1);
    if (p != FINGERPRINT_OK) return p;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BO TAY RA");
    Serial.println("Bo ngon tay ra...");
    delay(2000);

    while (finger.getImage() != FINGERPRINT_NOFINGER)
    {
        Blynk.run();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DAT LAI TAY");
    Serial.println("Dat lai cung ngon tay...");

    p = -1;
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        Blynk.run();
    }

    p = finger.image2Tz(2);
    if (p != FINGERPRINT_OK) return p;

    p = finger.createModel();
    if (p != FINGERPRINT_OK) return p;

    p = finger.storeModel(id);
    return p;
}

void addFingerprint()
{
    if (fireMode) return;

    finger.getTemplateCount();
    uint16_t id = finger.templateCount + 1;

    if (id > finger.capacity)
    {
        Serial.println("Bo nho van tay da day!");
        Blynk.virtualWrite(VP_STATUS, "Bo nho van tay da day");
        return;
    }

    Serial.print("Them van tay ID: ");
    Serial.println(id);

    uint8_t result = enrollFingerprint(id);

    if (result == FINGERPRINT_OK)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("THEM THANH CONG");
        lcd.setCursor(0, 1);
        lcd.print("ID: ");
        lcd.print(id);

        Blynk.virtualWrite(VP_STATUS, "Them van tay thanh cong");
    }
    else
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("THEM THAT BAI");

        Blynk.virtualWrite(VP_STATUS, "Them van tay that bai");
    }

    delay(1500);
}

// ================= VÂN TAY: XÓA GẦN NHẤT =================
void deleteLastFingerprint()
{
    if (fireMode) return;

    finger.getTemplateCount();

    if (finger.templateCount == 0)
    {
        Serial.println("Khong co van tay de xoa");
        Blynk.virtualWrite(VP_STATUS, "Khong co van tay de xoa");
        return;
    }

    uint16_t lastID = finger.templateCount;

    Serial.print("Dang xoa van tay ID: ");
    Serial.println(lastID);

    uint8_t result = finger.deleteModel(lastID);

    if (result == FINGERPRINT_OK)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DA XOA VAN TAY");
        lcd.setCursor(0, 1);
        lcd.print("ID: ");
        lcd.print(lastID);

        Blynk.virtualWrite(VP_STATUS, "Da xoa van tay gan nhat");
    }
    else
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("XOA THAT BAI");

        Blynk.virtualWrite(VP_STATUS, "Xoa van tay that bai");
    }

    delay(1500);
}

// ================= AUTO CLOSE =================
void autoCloseDoor()
{
    if (doorOpenedByFinger && millis() - doorOpenTime >= 5000)
    {
        closeDoor();
        doorOpenedByFinger = false;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DA DONG CUA");
        lcd.setCursor(0, 1);
        lcd.print("HE THONG OK");

        Blynk.virtualWrite(VP_STATUS, "Da dong cua");
    }
}

// ================= SETUP =================
void setup()
{
    Serial.begin(115200);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(FAN, OUTPUT);

    pinMode(BUTTON_ADD, INPUT_PULLUP);
    pinMode(BUTTON_DELETE, INPUT_PULLUP);

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(FAN, LOW);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    dht1.begin();
    dht2.begin();
    dht3.begin();

    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();

    lcd.setCursor(0, 0);
    lcd.print("KHOI DONG...");
    lcd.setCursor(0, 1);
    lcd.print("SMART HOME");

    doorServo.setPeriodHertz(50);
    doorServo.attach(DOOR, 500, 2400);
    closeDoor();

    FingerSerial.begin(57600, SERIAL_8N1, FINGER_RX, FINGER_TX);
    finger.begin(57600);

    if (finger.verifyPassword())
    {
        Serial.println("AS608 OK");
    }
    else
    {
        Serial.println("AS608 ERROR");
        lcd.clear();
        lcd.print("AS608 ERROR");
    }

    SimSerial.begin(115200, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    delay(1000);
    SimSerial.println("AT");
    delay(500);
    SimSerial.println("AT+CMGF=1");
    delay(500);

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    timer.setInterval(2000L, readDHT);
    timer.setInterval(2000L, updateLCD);
    timer.setInterval(500L, readGas);

    Blynk.virtualWrite(VP_STATUS, "He thong san sang");
}

// ================= LOOP =================
void loop()
{
    Blynk.run();
    timer.run();

    if (isButtonPressed(BUTTON_ADD))
    {
        addFingerprint();
    }

    if (isButtonPressed(BUTTON_DELETE))
    {
        deleteLastFingerprint();
    }

    checkFingerprint();
    autoCloseDoor();
    handleBuzzerTimer();
}