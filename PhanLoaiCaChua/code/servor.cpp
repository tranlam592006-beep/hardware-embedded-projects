// #include <Arduino.h>
// #include <ESP32Servo.h>

// Servo servoMG996R;

// #define SERVO_PIN 5

// void setup()
// {
//     Serial.begin(115200);

//     // Kết nối servo với GPIO 26
//     servoMG996R.attach(SERVO_PIN);

//     // Vị trí ban đầu
//     servoMG996R.write(0);
//     Serial.println("Servo o vi tri 0 do");
//     delay(2000);

//     // Quay đến 90 độ
//     servoMG996R.write(90);
//     Serial.println("Servo quay den 90 do");

//     // Giữ tại 90 độ trong 3 giây
//     delay(3000);

//     // Quay trở về vị trí ban đầu
//     servoMG996R.write(0);
//     Serial.println("Servo quay ve 0 do");

//     delay(1000);

//     // Ngừng phát xung điều khiển sau khi hoàn thành
//     servoMG996R.detach();
// }

// void loop()
// {
//     // Không lặp lại
// }