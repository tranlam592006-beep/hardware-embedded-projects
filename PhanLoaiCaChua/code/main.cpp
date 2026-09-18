#include <Arduino.h>
#include <ESP32Servo.h>
#include "HX711.h"


// =========================================================
// KHAI BÁO CHÂN
// =========================================================

// HX711
constexpr int HX711_DT_PIN  = 32;
constexpr int HX711_SCK_PIN = 33;

// Servo cấp quả lên băng tải
constexpr int FEEDER_SERVO_PIN = 5;

// Servo phân loại
constexpr int SERVO_1_PIN = 18;   // Green
constexpr int SERVO_2_PIN = 19;   // Ripe underweight
constexpr int SERVO_3_PIN = 23;   // Rotten

// Sensor vị trí
constexpr int SENSOR_1_PIN = 27;
constexpr int SENSOR_2_PIN = 26;
constexpr int SENSOR_3_PIN = 25;

// Điều khiển băng tải
constexpr int CONVEYOR_PIN = 17;


// =========================================================
// CẤU HÌNH LOADCELL
// =========================================================

// SỬA theo hệ số hiệu chuẩn thực tế của bạn
constexpr float CALIBRATION_FACTOR = -1080.930f;

// Dưới mức này coi như chưa có quả
constexpr float MIN_OBJECT_WEIGHT_G = 5.0f;

// Chín dưới 50 g => thiếu cân
constexpr float RIPE_MIN_WEIGHT_G = 75.0f;

// Cân coi là ổn định nếu thay đổi nhỏ hơn mức này
constexpr float WEIGHT_STABILITY_G = 2.0f;

// Cân phải ổn định liên tục trong thời gian này
constexpr unsigned long WEIGHT_STABLE_TIME_MS = 2000;

// Chu kỳ gửi cân nặng sang Python
constexpr unsigned long WEIGHT_SEND_INTERVAL_MS = 150;


// =========================================================
// CẤU HÌNH SERVO
// =========================================================

// Servo cấp
constexpr int FEEDER_READY_ANGLE = 0;
constexpr int FEEDER_PUSH_ANGLE  = 90;

// Servo phân loại
constexpr int SORT_READY_ANGLE = 90;
constexpr int SORT_PUSH_ANGLE  = 0;

// Thời gian servo cấp giữ ở 90 độ
constexpr unsigned long FEEDER_PUSH_TIME_MS = 2000;

// Sau khi servo cấp xong, giữ quả trước webcam 5 giây
constexpr unsigned long CAMERA_STABILIZE_TIME_MS = 1000;

// Servo phân loại giữ ở 30 độ
constexpr unsigned long SORT_PUSH_TIME_MS = 3000;

// Thời gian chờ sau khi servo quay lại 90
constexpr unsigned long SORT_RETURN_TIME_MS = 300;


// =========================================================
// CẤU HÌNH BĂNG TẢI
// =========================================================

// Chín đủ cân sẽ chạy thẳng đến cuối.
// SỬA thời gian này theo băng tải thực tế.
constexpr unsigned long END_TRAVEL_TIME_MS = 9000;

constexpr int CONVEYOR_ON  = HIGH;
constexpr int CONVEYOR_OFF = LOW;


// =========================================================
// CẤU HÌNH SENSOR
// =========================================================

// Nếu sensor của bạn phát hiện = LOW thì giữ LOW.
// Nếu phát hiện = HIGH thì đổi thành HIGH.
constexpr int SENSOR_ACTIVE_LEVEL = LOW;


// =========================================================
// ENUM TRẠNG THÁI
// =========================================================

enum class SystemState {
    WAITING_WEIGHT,
    FEEDING,
    WAITING_AI_RESULT,
    WAITING_SENSOR,
    SORTING,
    END_TRAVEL
};

enum class TomatoClass {
    NONE,
    GREEN,
    RIPE,
    ROTTEN
};

enum class SortDestination {
    NONE,
    SENSOR_1,
    SENSOR_2,
    SENSOR_3,
    END
};


// =========================================================
// ĐỐI TƯỢNG
// =========================================================

HX711 scale;

Servo feederServo;
Servo servo1;
Servo servo2;
Servo servo3;


// =========================================================
// BIẾN TOÀN CỤC
// =========================================================

SystemState systemState = SystemState::WAITING_WEIGHT;

TomatoClass tomatoClass = TomatoClass::NONE;
SortDestination destination = SortDestination::NONE;

float currentWeightG = 0.0f;
float previousWeightG = 0.0f;
float itemWeightG = 0.0f;

bool weightStabilityStarted = false;
unsigned long stableWeightStartMs = 0;
unsigned long lastWeightSendMs = 0;

unsigned long stateStartMs = 0;

bool aiResultReceived = false;

int activeServoNumber = 0;
bool sortServoReturned = false;


// =========================================================
// PROTOTYPE
// =========================================================

void sendWeightToPython(float weightG);

void processPythonSerial();
void processPythonCommand(const String &command);

void conveyorStart();
void conveyorStop();

void resetSystem();

void startFeeding();
void processFeeding();

void processWaitingWeight();
void processWaitingAIResult();
void processWaitingSensor();
void processSorting();
void processEndTravel();

bool sensorDetected(int sensorPin);

void startSortServo(int servoNumber);

void setState(SystemState newState);


// =========================================================
// SETUP
// =========================================================

void setup() {

    Serial.begin(115200);

    delay(500);


    // -----------------------------------------------------
    // SENSOR
    // GPIO 34, 35, 39 không có pull-up nội
    // -----------------------------------------------------

    pinMode(SENSOR_1_PIN, INPUT);
    pinMode(SENSOR_2_PIN, INPUT);
    pinMode(SENSOR_3_PIN, INPUT);


    // -----------------------------------------------------
    // BĂNG TẢI
    // -----------------------------------------------------

    pinMode(CONVEYOR_PIN, OUTPUT);

    conveyorStop();


    // -----------------------------------------------------
    // SERVO
    // -----------------------------------------------------

    feederServo.attach(FEEDER_SERVO_PIN);

    servo1.attach(SERVO_1_PIN);
    servo2.attach(SERVO_2_PIN);
    servo3.attach(SERVO_3_PIN);


    feederServo.write(FEEDER_READY_ANGLE);

    servo1.write(SORT_READY_ANGLE);
    servo2.write(SORT_READY_ANGLE);
    servo3.write(SORT_READY_ANGLE);


    // -----------------------------------------------------
    // HX711
    // -----------------------------------------------------

    scale.begin(
        HX711_DT_PIN,
        HX711_SCK_PIN
    );

    scale.set_scale(
        CALIBRATION_FACTOR
    );


    Serial.println("TARE_START");

    scale.tare();

    Serial.println("TARE_DONE");


    // -----------------------------------------------------
    // READY
    // -----------------------------------------------------

    Serial.println("ESP32_READY");
    Serial.println("STATE:WAITING_WEIGHT");
}


// =========================================================
// LOOP
// =========================================================

void loop() {

    // Luôn kiểm tra lệnh từ Python
    processPythonSerial();


    switch (systemState) {

        case SystemState::WAITING_WEIGHT:

            processWaitingWeight();

            break;


        case SystemState::FEEDING:

            processFeeding();

            break;


        case SystemState::WAITING_AI_RESULT:

            processWaitingAIResult();

            break;


        case SystemState::WAITING_SENSOR:

            processWaitingSensor();

            break;


        case SystemState::SORTING:

            processSorting();

            break;


        case SystemState::END_TRAVEL:

            processEndTravel();

            break;
    }
}


// =========================================================
// CHỜ QUẢ + ĐỌC CÂN
// =========================================================

void processWaitingWeight() {

    if (!scale.is_ready()) {
        return;
    }


    currentWeightG = scale.get_units(3);


    if (currentWeightG < 0.0f) {
        currentWeightG = 0.0f;
    }


    // -----------------------------------------------------
    // GỬI CÂN NẶNG SANG PYTHON
    // -----------------------------------------------------

    if (
        millis() - lastWeightSendMs
        >= WEIGHT_SEND_INTERVAL_MS
    ) {

        sendWeightToPython(
            currentWeightG
        );

        lastWeightSendMs = millis();
    }


    // -----------------------------------------------------
    // CHƯA CÓ QUẢ
    // -----------------------------------------------------

    if (
        currentWeightG
        < MIN_OBJECT_WEIGHT_G
    ) {

        previousWeightG = currentWeightG;

        weightStabilityStarted = false;

        stableWeightStartMs = 0;

        return;
    }


    // -----------------------------------------------------
    // KIỂM TRA CÂN ỔN ĐỊNH
    // -----------------------------------------------------

    float weightDifference = fabs(
        currentWeightG
        - previousWeightG
    );


    if (
        weightDifference
        <= WEIGHT_STABILITY_G
    ) {

        if (!weightStabilityStarted) {

            weightStabilityStarted = true;

            stableWeightStartMs = millis();
        }

        else if (
            millis() - stableWeightStartMs
            >= WEIGHT_STABLE_TIME_MS
        ) {

            // =============================================
            // CHỐT KHỐI LƯỢNG QUẢ
            // =============================================

            itemWeightG = currentWeightG;


            Serial.print("ITEM_WEIGHT:");
            Serial.println(
                itemWeightG,
                1
            );


            // =============================================
            // SERVO CẤP QUẢ
            // =============================================

            startFeeding();

            return;
        }
    }

    else {

        weightStabilityStarted = false;

        stableWeightStartMs = 0;
    }


    previousWeightG = currentWeightG;
}


// =========================================================
// BẮT ĐẦU SERVO CẤP
// =========================================================

void startFeeding() {

    conveyorStop();


    feederServo.write(
        FEEDER_PUSH_ANGLE
    );


    Serial.println(
        "STATE:FEEDING"
    );


    setState(
        SystemState::FEEDING
    );
}


// =========================================================
// XỬ LÝ SERVO CẤP
// =========================================================

void processFeeding() {

    if (
        millis() - stateStartMs
        < FEEDER_PUSH_TIME_MS
    ) {
        return;
    }


    // =====================================================
    // QUẢ ĐÃ ĐƯỢC GẠT VÀO BĂNG TẢI
    // NHƯNG BĂNG TẢI VẪN DỪNG
    // =====================================================

    conveyorStop();


    aiResultReceived = false;


    setState(
        SystemState::WAITING_AI_RESULT
    );


    // Báo Python bắt đầu nhận diện
    Serial.println(
        "STATE:WAITING_AI"
    );

    Serial.println(
        "CAMERA_STABILIZING"
    );
}


// =========================================================
// CHỜ 5 GIÂY + CHỜ AI
// =========================================================

void processWaitingAIResult() {

    // -----------------------------------------------------
    // Chưa đủ 5 giây
    // -----------------------------------------------------

    if (
        millis() - stateStartMs
        < CAMERA_STABILIZE_TIME_MS
    ) {

        return;
    }


    // -----------------------------------------------------
    // Đủ 5 giây nhưng AI chưa trả kết quả
    // -----------------------------------------------------

    if (!aiResultReceived) {

        return;
    }


    // =====================================================
    // ĐỦ 5 GIÂY + ĐÃ CÓ AI
    // BẮT ĐẦU CHẠY BĂNG TẢI
    // =====================================================

    conveyorStart();


    Serial.println(
        "CAMERA_DONE"
    );


    // -----------------------------------------------------
    // Chín đủ cân -> đi cuối băng tải
    // -----------------------------------------------------

    if (
        destination
        == SortDestination::END
    ) {

        setState(
            SystemState::END_TRAVEL
        );


        Serial.println(
            "STATE:END_TRAVEL"
        );
    }


    // -----------------------------------------------------
    // Green / Ripe thiếu cân / Rotten
    // -----------------------------------------------------

    else {

        setState(
            SystemState::WAITING_SENSOR
        );


        Serial.println(
            "STATE:WAITING_SENSOR"
        );
    }
}


// =========================================================
// ĐỌC SERIAL TỪ PYTHON
// =========================================================

void processPythonSerial() {

    while (
        Serial.available() > 0
    ) {

        String command =
            Serial.readStringUntil('\n');


        command.trim();


        if (
            command.length() > 0
        ) {

            processPythonCommand(
                command
            );
        }
    }
}


// =========================================================
// XỬ LÝ CLASS TỪ PYTHON
// =========================================================

void processPythonCommand(
    const String &command
) {

    // -----------------------------------------------------
    // Chỉ nhận AI khi đang chờ
    // -----------------------------------------------------

    if (
        systemState
        != SystemState::WAITING_AI_RESULT
    ) {

        Serial.print(
            "COMMAND_IGNORED:"
        );

        Serial.println(
            command
        );

        return;
    }


    // =====================================================
    // GREEN
    // =====================================================

    if (
        command
        == "CLASS:GREEN"
    ) {

        tomatoClass =
            TomatoClass::GREEN;


        destination =
            SortDestination::SENSOR_1;


        Serial.println(
            "CLASS_ACCEPTED:GREEN"
        );


        Serial.println(
            "ROUTE:SENSOR_1"
        );
    }


    // =====================================================
    // RIPE
    // =====================================================

    else if (
        command
        == "CLASS:RIPE"
    ) {

        tomatoClass =
            TomatoClass::RIPE;


        Serial.println(
            "CLASS_ACCEPTED:RIPE"
        );


        // -------------------------------------------------
        // Chín thiếu cân
        // -------------------------------------------------

        if (
            itemWeightG
            < RIPE_MIN_WEIGHT_G
        ) {

            destination =
                SortDestination::SENSOR_2;


            Serial.println(
                "ROUTE:RIPE_UNDERWEIGHT"
            );
        }


        // -------------------------------------------------
        // Chín đủ cân
        // -------------------------------------------------

        else {

            destination =
                SortDestination::END;


            Serial.println(
                "ROUTE:RIPE_OK"
            );
        }
    }


    // =====================================================
    // ROTTEN
    // =====================================================

    else if (
        command
        == "CLASS:ROTTEN"
    ) {

        tomatoClass =
            TomatoClass::ROTTEN;


        destination =
            SortDestination::SENSOR_3;


        Serial.println(
            "CLASS_ACCEPTED:ROTTEN"
        );


        Serial.println(
            "ROUTE:SENSOR_3"
        );
    }


    // =====================================================
    // COMMAND SAI
    // =====================================================

    else {

        Serial.print(
            "UNKNOWN_COMMAND:"
        );

        Serial.println(
            command
        );

        return;
    }


    // =====================================================
    // GHI NHẬN ĐÃ CÓ KẾT QUẢ AI
    // =====================================================

    aiResultReceived = true;


    Serial.println(
        "AI_RESULT_READY"
    );
}


// =========================================================
// SENSOR CÓ PHÁT HIỆN KHÔNG
// =========================================================

bool sensorDetected(
    int sensorPin
) {

    return (
        digitalRead(sensorPin)
        == SENSOR_ACTIVE_LEVEL
    );
}


// =========================================================
// CHỜ SENSOR
// =========================================================

void processWaitingSensor() {

    // LƯU Ý:
    // Băng tải vẫn tiếp tục chạy


    // -----------------------------------------------------
    // GREEN -> SENSOR 1
    // -----------------------------------------------------

    if (
        destination
        == SortDestination::SENSOR_1
        &&
        sensorDetected(
            SENSOR_1_PIN
        )
    ) {

        Serial.println(
            "SENSOR:1"
        );


        startSortServo(
            1
        );
    }


    // -----------------------------------------------------
    // RIPE UNDERWEIGHT -> SENSOR 2
    // -----------------------------------------------------

    else if (
        destination
        == SortDestination::SENSOR_2
        &&
        sensorDetected(
            SENSOR_2_PIN
        )
    ) {

        Serial.println(
            "SENSOR:2"
        );


        startSortServo(
            2
        );
    }


    // -----------------------------------------------------
    // ROTTEN -> SENSOR 3
    // -----------------------------------------------------

    else if (
        destination
        == SortDestination::SENSOR_3
        &&
        sensorDetected(
            SENSOR_3_PIN
        )
    ) {

        Serial.println(
            "SENSOR:3"
        );


        startSortServo(
            3
        );
    }
}


// =========================================================
// BẮT ĐẦU GẠT SERVO
// =========================================================

void startSortServo(int servoNumber) {

    activeServoNumber = servoNumber;
    sortServoReturned = false;


    // -----------------------------------------------------
    // Servo 1
    // -----------------------------------------------------

    if ( servoNumber == 1) {
        servo1.write(
            SORT_PUSH_ANGLE
        );
    }


    // -----------------------------------------------------
    // Servo 2
    // -----------------------------------------------------

    else if (
        servoNumber == 2
    ) {

        servo2.write(
            SORT_PUSH_ANGLE
        );
    }


    // -----------------------------------------------------
    // Servo 3
    // -----------------------------------------------------

    else if (
        servoNumber == 3
    ) {

        servo3.write(
            SORT_PUSH_ANGLE
        );
    }


    // BĂNG TẢI KHÔNG DỪNG


    setState(
        SystemState::SORTING
    );
}


// =========================================================
// SERVO GẠT
// =========================================================

void processSorting() {

    unsigned long elapsed =
        millis() - stateStartMs;


    // -----------------------------------------------------
    // ĐANG GIỮ SERVO Ở 30°
    // -----------------------------------------------------

    if (
        !sortServoReturned
        &&
        elapsed
        >= SORT_PUSH_TIME_MS
    ) {

        // =============================================
        // Quay lại 90°
        // =============================================

        if (
            activeServoNumber == 1
        ) {

            servo1.write(
                SORT_READY_ANGLE
            );
        }

        else if (
            activeServoNumber == 2
        ) {

            servo2.write(
                SORT_READY_ANGLE
            );
        }

        else if (
            activeServoNumber == 3
        ) {

            servo3.write(
                SORT_READY_ANGLE
            );
        }


        sortServoReturned = true;


        stateStartMs =
            millis();


        return;
    }


    // -----------------------------------------------------
    // SERVO ĐÃ VỀ 90°
    // -----------------------------------------------------

    if (
        sortServoReturned
        &&
        millis() - stateStartMs
        >= SORT_RETURN_TIME_MS
    ) {

        Serial.println(
            "SORT:DONE"
        );


        resetSystem();
    }
}


// =========================================================
// CHÍN ĐỦ CÂN -> CHẠY HẾT BĂNG TẢI
// =========================================================

void processEndTravel() {

    // Băng tải vẫn chạy


    if (
        millis() - stateStartMs
        >= END_TRAVEL_TIME_MS
    ) {

        Serial.println(
            "END_REACHED"
        );


        resetSystem();
    }
}


// =========================================================
// BĂNG TẢI
// =========================================================

void conveyorStart() {

    digitalWrite(
        CONVEYOR_PIN,
        CONVEYOR_ON
    );


    Serial.println(
        "CONVEYOR:ON"
    );
}


void conveyorStop() {

    digitalWrite(
        CONVEYOR_PIN,
        CONVEYOR_OFF
    );


    Serial.println(
        "CONVEYOR:OFF"
    );
}


// =========================================================
// RESET CHU TRÌNH
// =========================================================

void resetSystem() {

    conveyorStop();


    // -----------------------------------------------------
    // Servo về vị trí ban đầu
    // -----------------------------------------------------

    feederServo.write(
        FEEDER_READY_ANGLE
    );


    servo1.write(
        SORT_READY_ANGLE
    );

    servo2.write(
        SORT_READY_ANGLE
    );

    servo3.write(
        SORT_READY_ANGLE
    );


    // -----------------------------------------------------
    // Reset biến
    // -----------------------------------------------------

    currentWeightG = 0.0f;

    previousWeightG = 0.0f;

    itemWeightG = 0.0f;


    tomatoClass =
        TomatoClass::NONE;


    destination =
        SortDestination::NONE;


    weightStabilityStarted =
        false;


    stableWeightStartMs = 0;


    aiResultReceived =
        false;


    activeServoNumber = 0;


    sortServoReturned =
        false;


    // -----------------------------------------------------
    // Báo Python
    // -----------------------------------------------------

    Serial.println(
        "CYCLE:RESET"
    );


    setState(
        SystemState::WAITING_WEIGHT
    );


    Serial.println(
        "STATE:WAITING_WEIGHT"
    );
}


// =========================================================
// SET STATE
// =========================================================

void setState(
    SystemState newState
) {

    systemState =
        newState;


    stateStartMs =
        millis();
}


// =========================================================
// GỬI KHỐI LƯỢNG SANG PYTHON
// =========================================================

void sendWeightToPython(
    float weightG
) {

    Serial.print(
        "WEIGHT:"
    );


    Serial.println(
        weightG,
        1
    );
}