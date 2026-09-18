// #include <Arduino.h>
// #include <Adafruit_Fingerprint.h>

// #define BUTTON_ADD     15
// #define BUTTON_DELETE  13

// #define FINGER_RX 19   // ESP32 RX <- TX AS608
// #define FINGER_TX 18   // ESP32 TX -> RX AS608

// HardwareSerial FingerSerial(1);
// Adafruit_Fingerprint finger = Adafruit_Fingerprint(&FingerSerial);

// uint8_t enrollFingerprint(uint16_t id)
// {
//     int p = -1;

//     Serial.println("Dat ngon tay lan 1...");

//     while (p != FINGERPRINT_OK)
//     {
//         p = finger.getImage();
//     }

//     p = finger.image2Tz(1);
//     if (p != FINGERPRINT_OK) return p;

//     Serial.println("Bo ngon tay ra...");
//     delay(2000);

//     while (finger.getImage() != FINGERPRINT_NOFINGER);

//     Serial.println("Dat lai cung ngon tay...");

//     p = -1;
//     while (p != FINGERPRINT_OK)
//     {
//         p = finger.getImage();
//     }

//     p = finger.image2Tz(2);
//     if (p != FINGERPRINT_OK) return p;

//     Serial.println("Dang tao mau van tay...");
//     p = finger.createModel();
//     if (p != FINGERPRINT_OK) return p;

//     Serial.print("Dang luu vao ID ");
//     Serial.println(id);

//     p = finger.storeModel(id);
//     return p;
// }

// void addFingerprint()
// {
//     finger.getTemplateCount();

//     uint16_t id = finger.templateCount + 1;

//     if (id > finger.capacity)
//     {
//         Serial.println("Bo nho van tay da day!");
//         return;
//     }

//     Serial.print("So van tay hien co: ");
//     Serial.println(finger.templateCount);

//     Serial.print("Dang ky van tay moi voi ID: ");
//     Serial.println(id);

//     uint8_t result = enrollFingerprint(id);

//     if (result == FINGERPRINT_OK)
//     {
//         Serial.println("Them van tay thanh cong!");
//     }
//     else
//     {
//         Serial.print("Them van tay that bai, ma loi: ");
//         Serial.println(result);
//     }
// }

// void deleteAllFingerprints()
// {
//     Serial.println("Dang xoa tat ca van tay...");

//     uint8_t result = finger.emptyDatabase();

//     if (result == FINGERPRINT_OK)
//     {
//         Serial.println("Da xoa toan bo van tay!");
//     }
//     else
//     {
//         Serial.print("Xoa that bai, ma loi: ");
//         Serial.println(result);
//     }
// }

// bool isButtonPressed(int pin)
// {
//     if (digitalRead(pin) == LOW)
//     {
//         delay(50);
//         if (digitalRead(pin) == LOW)
//         {
//             while (digitalRead(pin) == LOW);
//             delay(50);
//             return true;
//         }
//     }

//     return false;
// }

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(BUTTON_ADD, INPUT_PULLUP);
//     pinMode(BUTTON_DELETE, INPUT_PULLUP);

//     FingerSerial.begin(57600, SERIAL_8N1, FINGER_RX, FINGER_TX);
//     finger.begin(57600);

//     Serial.println("===== TEST AS608 =====");

//     if (finger.verifyPassword())
//     {
//         Serial.println("AS608 ket noi thanh cong!");
//     }
//     else
//     {
//         Serial.println("Khong tim thay AS608!");
//         while (1);
//     }

//     finger.getTemplateCount();

//     Serial.print("Dung luong toi da: ");
//     Serial.println(finger.capacity);

//     Serial.print("So van tay da luu: ");
//     Serial.println(finger.templateCount);

//     Serial.println("BUTTON1: Them van tay");
//     Serial.println("BUTTON2: Xoa tat ca van tay");
// }

// void loop()
// {
//     if (isButtonPressed(BUTTON_ADD))
//     {
//         addFingerprint();
//     }

//     if (isButtonPressed(BUTTON_DELETE))
//     {
//         deleteAllFingerprints();
//     }
// }