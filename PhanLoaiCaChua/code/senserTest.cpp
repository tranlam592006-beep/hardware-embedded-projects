// #include <Arduino.h>

// // Khai báo chân OUT của 3 cảm biến hồng ngoại
// const uint8_t IR_SENSOR_1_PIN = 27;
// const uint8_t IR_SENSOR_2_PIN = 26;
// const uint8_t IR_SENSOR_3_PIN = 25;

// // Phần lớn module IR xuất LOW khi phát hiện vật
// const uint8_t OBJECT_DETECTED_LEVEL = LOW;

// void setup()
// {
//     Serial.begin(115200);

//     pinMode(IR_SENSOR_1_PIN, INPUT);
//     pinMode(IR_SENSOR_2_PIN, INPUT);
//     pinMode(IR_SENSOR_3_PIN, INPUT);

//     delay(1000);

//     Serial.println();
//     Serial.println("===== TEST 3 CAM BIEN HONG NGOAI =====");
//     Serial.println("LOW  = Co vat can");
//     Serial.println("HIGH = Khong co vat can");
//     Serial.println("-------------------------------------");
// }

// void loop()
// {
//     // Đọc tín hiệu từ cảm biến
//     int sensor1State = digitalRead(IR_SENSOR_1_PIN);
//     int sensor2State = digitalRead(IR_SENSOR_2_PIN);
//     int sensor3State = digitalRead(IR_SENSOR_3_PIN);

//     bool sensor1Detected = (sensor1State == OBJECT_DETECTED_LEVEL);
//     bool sensor2Detected = (sensor2State == OBJECT_DETECTED_LEVEL);
//     bool sensor3Detected = (sensor3State == OBJECT_DETECTED_LEVEL);

//     Serial.print("Cam bien 1: ");
//     Serial.print(sensor1Detected ? "CO VAT" : "KHONG CO VAT");

//     Serial.print(" | Cam bien 2: ");
//     Serial.print(sensor2Detected ? "CO VAT" : "KHONG CO VAT");

//     Serial.print(" | Cam bien 3: ");
//     Serial.println(sensor3Detected ? "CO VAT" : "KHONG CO VAT");

//     delay(300);
// }