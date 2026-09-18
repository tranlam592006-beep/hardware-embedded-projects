// #include <Arduino.h>

// #define SIM_RX_PIN 16   // ESP32 RX2 <- TX SIM
// #define SIM_TX_PIN 17   // ESP32 TX2 -> RX SIM

// HardwareSerial SimSerial(2);

// String ADMIN_PHONE = "+84356452054";

// void readSIM(unsigned long timeout = 3000)
// {
//     unsigned long start = millis();

//     while (millis() - start < timeout)
//     {
//         while (SimSerial.available())
//         {
//             Serial.write(SimSerial.read());
//         }
//     }
// }

// void sendAT(String cmd, unsigned long waitTime = 3000)
// {
//     Serial.print("Gui lenh: ");
//     Serial.println(cmd);

//     SimSerial.println(cmd);
//     readSIM(waitTime);

//     Serial.println("--------------------");
// }

// void sendSMS(String phone, String message)
// {
//     Serial.println("===== TEST GUI SMS =====");

//     sendAT("AT", 2000);
//     sendAT("AT+CMGF=1", 2000);
//     sendAT("AT+CSCS=\"GSM\"", 2000);

//     Serial.println("Gui lenh CMGS...");
//     SimSerial.print("AT+CMGS=\"");
//     SimSerial.print(phone);
//     SimSerial.println("\"");

//     readSIM(3000);

//     SimSerial.print(message);
//     SimSerial.write(26);   // Ctrl + Z

//     readSIM(10000);

//     Serial.println("===== KET THUC SMS =====");
// }

// void setup()
// {
//     Serial.begin(115200);

//     SimSerial.begin(115200, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);

//     Serial.println("===== TEST MODULE SIM 4G =====");
//     delay(3000);

//     sendAT("AT", 2000);          // Kiểm tra module có phản hồi không
//     sendAT("ATI", 3000);         // Thông tin module
//     sendAT("AT+CPIN?", 3000);    // Kiểm tra SIM
//     sendAT("AT+CSQ", 3000);      // Kiểm tra sóng
//     sendAT("AT+COPS?", 3000);    // Nhà mạng
//     sendAT("AT+CREG?", 3000);    // Đăng ký mạng 2G/3G
//     sendAT("AT+CEREG?", 3000);   // Đăng ký mạng LTE/4G

//     sendSMS(ADMIN_PHONE, "TEST SIM 4G: Module da gui SMS thanh cong.");
// }

// void loop()
// {
//     // Cho phep go lenh AT truc tiep tu Serial Monitor
//     if (Serial.available())
//     {
//         SimSerial.write(Serial.read());
//     }

//     if (SimSerial.available())
//     {
//         Serial.write(SimSerial.read());
//     }
// }