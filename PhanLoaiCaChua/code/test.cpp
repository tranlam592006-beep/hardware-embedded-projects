// #include <Arduino.h>

// void setup()
// {
//     Serial.begin(115200);

//     Serial.println("ESP32 READY");
// }

// void loop()
// {
//     if (Serial.available())
//     {
//         String data = Serial.readStringUntil('\n');

//         data.trim();

//         if (data == "GREEN")
//         {
//             Serial.println("Nhan duoc: GREEN");
//         }

//         else if (data == "RIPE")
//         {
//             Serial.println("Nhan duoc: RIPE");
//         }

//         else if (data == "ROTTEN")
//         {
//             Serial.println("Nhan duoc: ROTTEN");
//         }
//     }
// }