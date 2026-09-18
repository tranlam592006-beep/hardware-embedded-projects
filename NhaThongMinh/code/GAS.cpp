// #include <Arduino.h>

// //================= KHAI BÁO CHÂN =================//

// #define MQ2_SENSOR1 34   // Phòng 1
// #define MQ2_SENSOR2 39   // Phòng 2
// #define MQ2_SENSOR3 36   // Phòng 3

// int gas1 = 0;
// int gas2 = 0;
// int gas3 = 0;

// //================= HÀM ĐỌC CẢM BIẾN =================//

// const unsigned long MQ2_WARMUP_TIME = 60000; // 60 giay

// void readGas()
// {
//     if (millis() < MQ2_WARMUP_TIME)
//     {
//         Serial.println("MQ2 dang lam nong, bo qua du lieu...");
//         return;
//     }

//     gas1 = analogRead(MQ2_SENSOR1);
//     gas2 = analogRead(MQ2_SENSOR2);
//     gas3 = analogRead(MQ2_SENSOR3);

//     Serial.print("Gas1: ");
//     Serial.print(gas1);
//     Serial.print(" | Gas2: ");
//     Serial.print(gas2);
//     Serial.print(" | Gas3: ");
//     Serial.println(gas3);
// }

//================= SETUP =================//

// void setup()
// {
//     Serial.begin(115200);

//     // Cấu hình ADC ESP32
//     analogReadResolution(12);          // Giá trị từ 0 -> 4095
//     analogSetAttenuation(ADC_11db);    // Dải đo khoảng 0 -> 3.3V

//     pinMode(MQ2_SENSOR1, INPUT);
//     pinMode(MQ2_SENSOR2, INPUT);
//     pinMode(MQ2_SENSOR3, INPUT);

//     Serial.println("===== TEST 3 CAM BIEN MQ2 =====");
// }

// //================= LOOP =================//

// void loop()
// {
//     readGas();

//     delay(1000);   // Đọc mỗi 1 giây
// }