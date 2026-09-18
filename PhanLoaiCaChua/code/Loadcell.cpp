
// #include <Arduino.h>
// #include <HX711.h>

// constexpr uint8_t HX711_DT_PIN = 32;
// constexpr uint8_t HX711_SCK_PIN = 33;

// HX711 scale;

// // Khối lượng quả cân chuẩn bạn đang dùng, đơn vị gram
// constexpr float KNOWN_WEIGHT_G = 100.0F;

// void setup() {
//     Serial.begin(115200);
//     delay(1000);

//     Serial.println();
//     Serial.println("======================================");
//     Serial.println("HIEU CHUAN LOADCELL + HX711");
//     Serial.println("======================================");

//     scale.begin(HX711_DT_PIN, HX711_SCK_PIN);

//     Serial.println("Dang kiem tra HX711...");

//     unsigned long startTime = millis();

//     while (!scale.is_ready()) {
//         if (millis() - startTime > 5000) {
//             Serial.println("LOI: HX711 khong phan hoi.");
//             Serial.println("Kiem tra DT, SCK, nguon va day loadcell.");
//             while (true) {
//                 delay(1000);
//             }
//         }

//         delay(100);
//     }

//     Serial.println("HX711 da san sang.");
//     Serial.println();
//     Serial.println("Buoc 1: Bo het vat khoi can.");
//     Serial.println("Nhan phim T roi Enter de tru bi.");
// }

// void loop() {
//     if (!Serial.available()) {
//         return;
//     }

//     char command = Serial.read();

//     // Xóa các ký tự Enter còn lại
//     while (Serial.available()) {
//         Serial.read();
//     }

//     if (command == 't' || command == 'T') {
//         Serial.println();
//         Serial.println("Dang tru bi, khong cham vao can...");

//         // Chưa dùng hệ số hiệu chuẩn
//         scale.set_scale();
//         scale.tare(20);

//         Serial.println("Da tru bi.");
//         Serial.println();
//         Serial.print("Buoc 2: Dat vat chuan ");
//         Serial.print(KNOWN_WEIGHT_G, 1);
//         Serial.println(" g len can.");
//         Serial.println("Cho can on dinh, sau do nhan C roi Enter.");
//     }

//     else if (command == 'c' || command == 'C') {
//         if (!scale.is_ready()) {
//             Serial.println("HX711 chua san sang.");
//             return;
//         }

//         Serial.println();
//         Serial.println("Dang lay gia tri trung binh...");
//         delay(1000);

//         // Giá trị raw sau khi đã trừ bì
//         float rawValue = scale.get_value(30);

//         float calibrationFactor = rawValue / KNOWN_WEIGHT_G;

//         Serial.println("======================================");
//         Serial.print("Gia tri raw: ");
//         Serial.println(rawValue, 2);

//         Serial.print("Khoi luong chuan: ");
//         Serial.print(KNOWN_WEIGHT_G, 1);
//         Serial.println(" g");

//         Serial.print("CALIBRATION_FACTOR = ");
//         Serial.println(calibrationFactor, 4);
//         Serial.println("======================================");

//         scale.set_scale(calibrationFactor);

//         Serial.print("Can kiem tra lai: ");
//         Serial.print(scale.get_units(20), 1);
//         Serial.println(" g");

//         Serial.println();
//         Serial.println("Nhan R roi Enter de doc can lien tuc.");
//         Serial.println("Nhan T roi Enter de tru bi lai.");
//     }

//     else if (command == 'r' || command == 'R') {
//         Serial.println();
//         Serial.println("Dang doc can trong 10 giay...");

//         for (int i = 0; i < 50; i++) {
//             if (scale.is_ready()) {
//                 float weight = scale.get_units(10);

//                 if (weight > -0.5F && weight < 0.5F) {
//                     weight = 0.0F;
//                 }

//                 Serial.print("Khoi luong: ");
//                 Serial.print(weight, 1);
//                 Serial.println(" g");
//             } else {
//                 Serial.println("HX711 khong san sang.");
//             }

//             delay(200);
//         }

//         Serial.println("Da dung doc lien tuc.");
//     }
// }