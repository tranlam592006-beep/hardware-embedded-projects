
#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <Preferences.h>

// =================== KHAI BÁO CHÂN =====================

Preferences prefs;

// SRF05
#define TRIG_PIN 19
#define ECHO_PIN 21

// Buzzer
#define BUZZER_PIN 4
#define BUZZER_CHANNEL 0

// Motor rung
#define MOTOR_PIN 23
#define VIB_CHANNEL 1

// GPS NEO-6M
#define GPS_RX_PIN 18    // ESP32 RX nối TX GPS
#define GPS_TX_PIN 5     // ESP32 TX nối RX GPS

// SIM A7680C
#define SIM_RX_PIN 32    // ESP32 RX nối TX SIM
#define SIM_TX_PIN 33    // ESP32 TX nối RX SIM


// =================== UART ===================

HardwareSerial SIM_Serial(1);
HardwareSerial GPS_Serial(2);

TinyGPSPlus gps;

// =================== CẤU HÌNH ===================

String ADMIN_PHONE = "+84356452054";   // đổi thành số điện thoại của bạn
String DEFAULT_ADMIN_PHONE = "+84356452054";

#define GPS_BAUD 9600
#define SIM_BAUD 115200

#define DANGER_DISTANCE 50       // cm, quá gần thì gửi SMS cảnh báo
#define MAX_DISTANCE 200         // cm, ngoài khoảng này không cảnh báo

#define STABLE_DELTA 10           // cm, thay đổi nhỏ hơn mức này coi như không đáng kể
#define STABLE_TIME 5000        // ms, đứng yên lâu thì tắt cảnh báo

#define SMS_COOLDOWN 60000       // ms, chống spam SMS

// =================== BIẾN HỆ THỐNG ===================

float distanceCm = 999;
float lastDistanceCm = 999;

unsigned long lastChangeTime = 0;
unsigned long lastSmsWarningTime = 0;

bool objectStable = false;

#define GPS_UPDATE_INTERVAL 1800000UL   // 30 phút

double savedLat = 0.0;
double savedLng = 0.0;

bool hasSavedLocation = false;

unsigned long lastGpsSaveTime = 0;


// lưu vị trí
void updateSavedLocation()
{
    if (gps.location.isValid() && gps.location.isUpdated())
    {
        unsigned long now = millis();

        if (!hasSavedLocation || now - lastGpsSaveTime >= GPS_UPDATE_INTERVAL)
        {
            savedLat = gps.location.lat();
            savedLng = gps.location.lng();

            hasSavedLocation = true;
            lastGpsSaveTime = now;

            Serial.println("Da luu vi tri GPS:");
            Serial.print("Lat: ");
            Serial.println(savedLat, 6);
            Serial.print("Lng: ");
            Serial.println(savedLng, 6);
        }
    }
}


// =================== GỬI SMS ===================

void sendSMS(String phone, String message)
{
    SIM_Serial.println("AT+CMGF=1");  // chuyển sang chế độ SMS dạng text
    delay(500);

    SIM_Serial.print("AT+CMGS=\"");   // Đây là phần đầu của lệnh gửi SMS
    SIM_Serial.print(phone);
    SIM_Serial.println("\"");
    delay(500);  //  gửi SMS đến số phone

    SIM_Serial.print(message);
    delay(300);

    SIM_Serial.write(26);   // Ctrl + Z: nhất núi gửi
    delay(5000);
}

// =================== LƯU SỐ ĐIỆN THOẠI ================
void loadAdminPhone()
{
    prefs.begin("config", false);

    ADMIN_PHONE = prefs.getString("admin", DEFAULT_ADMIN_PHONE);

    Serial.print("So admin hien tai: ");
    Serial.println(ADMIN_PHONE);
}

void saveAdminPhone(String newPhone)
{
    newPhone.trim();

    if (newPhone.length() >= 10 && newPhone.startsWith("+"))
    {
        ADMIN_PHONE = newPhone;

        prefs.putString("admin", ADMIN_PHONE);

        Serial.print("Da luu so admin moi: ");
        Serial.println(ADMIN_PHONE);

        sendSMS(ADMIN_PHONE,
                "Da cap nhat so admin moi:\n" + ADMIN_PHONE);
    }
    else
    {
        sendSMS(ADMIN_PHONE,
                "Sai dinh dang so dien thoai.\nVi du: ADMIN:+84912345678");
    }
}


void updateAdminPhone(String smsText)
{
    smsText.trim();

    String upperText = smsText;
    upperText.toUpperCase();

    if (upperText.startsWith("ADMIN:"))
    {
        String newPhone = smsText.substring(6);
        newPhone.trim();

        saveAdminPhone(newPhone);
    }
}

// =================== ĐỌC KHOẢNG CÁCH SRF05 ===================

float readDistanceCm()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0)
    {
        return 999;
    }

    float distance = duration * 0.0343 / 2.0;
    return distance;
}

//=================== PWM CẢNH BÁO ===================

void setWarningLevel(float distance)
{
    if (distance > MAX_DISTANCE || objectStable)
    {
        ledcWrite(BUZZER_CHANNEL, 0);
        ledcWrite(VIB_CHANNEL, 0);
        return;
    }

    int pwmValue = map(distance, MAX_DISTANCE, DANGER_DISTANCE, 50, 255);

    // PWM riêng cho motor (20 -> 90)
    int vibValue = map(distance, MAX_DISTANCE, DANGER_DISTANCE, 20, 60);

    pwmValue = constrain(pwmValue, 0, 255);

    if (distance <= DANGER_DISTANCE)
    {
        pwmValue = 255;
    }

    ledcWrite(BUZZER_CHANNEL, pwmValue);
    ledcWrite(VIB_CHANNEL, vibValue);
}

// =================== GỬI LỆNH AT ===================

// void sendAT(String cmd, int waitTime = 500)
// {
//     SIM_Serial.println(cmd);
//     delay(waitTime);

//     while (SIM_Serial.available())
//     {
//         Serial.write(SIM_Serial.read());
//     }
// }

void sendAT(String cmd)
{
    Serial.println(">> " + cmd);

    SIM_Serial.println(cmd);

    unsigned long start = millis();

    while (millis() - start < 3000)
    {
        while (SIM_Serial.available())
        {
            Serial.write(SIM_Serial.read());
        }
    }
}


// =================== LẤY VỊ TRÍ GPS ===================

String getLocationMessage()
{
    if (hasSavedLocation)
    {
        String msg = "Vi tri hien tai:\n";
        msg += "Lat: ";
        msg += String(savedLat, 6); // 6: chữ số sau dấu phẩy
        msg += "\nLng: ";
        msg += String(savedLng, 6);
        msg += "\nGoogle Maps:\n";
        msg += "https://maps.google.com/?q=";
        msg += String(savedLat, 6);
        msg += ",";
        msg += String(savedLng, 6);
        return msg;
    }
    else
    {
        return "Vi tri hien tai: Lat:20.9848351 Lng:105.7987384. https://maps.google.com/?q=20.984835,105.798738";
    }
}

// String getLocationMessage()
// {
//         String msg = "Vi tri hien tai:\n";
//         msg += "Lat: ";
//         msg += String(20.9848351, 6); // 6: chữ số sau dấu phẩy
//         msg += "\nLng: ";
//         msg += String(105.7987384, 6);
//         msg += "\nGoogle Maps:\n";
//         msg += "https://maps.google.com/?q=";
//         msg += String(20.9848351, 6);
//         msg += ",";
//         msg += String(105.7987384, 6);
//         return msg;
// }

// // Tọa độ mặc định
// const float DEFAULT_LAT = 20.9848351;   
// const float DEFAULT_LNG = 105.7987384;

// String getLocationMessage()
// {
//     float lat, lng;

//     if (hasSavedLocation)
//     {
//         // Dùng tọa độ GPS thực tế
//         lat = savedLat;
//         lng = savedLng;
//     }
//     else
//     {
//         // Dùng tọa độ cố định khi mất GPS
//         lat = DEFAULT_LAT;
//         lng = DEFAULT_LNG;
//     }

//     String msg = "Vi tri hien tai:\n";
//     msg += "Lat: ";
//     msg += String(lat, 6);
//     msg += "\nLng: ";
//     msg += String(lng, 6);
//     msg += "\nGoogle Maps:\n";
//     msg += "https://maps.google.com/?q=";
//     msg += String(lat, 6);
//     msg += ",";
//     msg += String(lng, 6);

//     return msg;
// }

// =================== XỬ LÝ TIN NHẮN ĐẾN ===================

void checkSMS()
{
    static String simData = ""; // static là để lưu dữ liệu, không bị mất khi gọi lại hàm check này

    while (SIM_Serial.available()) // kiếm tra dữ liệu từ sim
    {
        char c = SIM_Serial.read();
        simData += c;

        if (c == '\n')
        {
            simData.trim();     // xóa khoảng trắng

            Serial.println(simData);

            if (simData.indexOf("LOC") >= 0 ||
                simData.indexOf("GPS") >= 0 ||
                simData.indexOf("VITRI") >= 0)
            {
                sendSMS(ADMIN_PHONE, getLocationMessage());
            }
            updateAdminPhone(simData);

            simData = "";
        }
    }
}

// =================== ĐỌC GPS ===================

void readGPS() // độc từng kí tự một rồi gửi dịch ra định vị
{
    while (GPS_Serial.available()) // kiểm tra xem có dữ liệu chưa
    {
        gps.encode(GPS_Serial.read());  
    }
}

// =================== KIỂM TRA VẬT CẢN ĐỨNG YÊN ===================

void checkStableObject()
{
    if (abs(distanceCm - lastDistanceCm) > STABLE_DELTA)
    {
        lastDistanceCm = distanceCm;
        lastChangeTime = millis();
        objectStable = false;
    }

    if (millis() - lastChangeTime > STABLE_TIME)
    {
        objectStable = true;
    }
}

// =================== SETUP ===================

void setup()
{
    Serial.begin(115200);
    loadAdminPhone();

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // PWM buzzer
    ledcSetup(BUZZER_CHANNEL, 2000, 8);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

    // PWM motor rung
    ledcSetup(VIB_CHANNEL, 200, 8);
    ledcAttachPin(MOTOR_PIN, VIB_CHANNEL);

    ledcWrite(BUZZER_CHANNEL, 0);
    ledcWrite(VIB_CHANNEL, 0);

    // UART GPS
    GPS_Serial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    // UART SIM
    SIM_Serial.begin(SIM_BAUD, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);

    delay(3000);

    sendAT("AT");
    sendAT("ATE0");
    sendAT("AT+CMGF=1");
    sendAT("AT+CNMI=2,2,0,0,0");

    lastChangeTime = millis();

    Serial.println("He thong canh bao va cham da san sang");
}

void loop()
{
    readGPS();
    updateSavedLocation();
    checkSMS();

    distanceCm = readDistanceCm();

    Serial.print("Distance: ");
    Serial.print(distanceCm);
    Serial.println(" cm");

    checkStableObject();
    setWarningLevel(distanceCm);

    if (distanceCm <= DANGER_DISTANCE && !objectStable)
    {
        if (millis() - lastSmsWarningTime > SMS_COOLDOWN)
        {
            String msg = "CANH BAO! Co nguy co xay ra tai nan.\n";
            msg += "Khoang cach: ";
            msg += String(distanceCm);
            msg += " cm\n\n";
            msg += getLocationMessage();

            sendSMS(ADMIN_PHONE, msg);

            lastSmsWarningTime = millis();
        }
    }

    delay(100);
}


