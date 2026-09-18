#include <Arduino.h>

const int NUM_STAIRS = 15;

const uint8_t sensorPins[NUM_STAIRS] = {
  42, 1, 0, 44, 43,
  41, 40, 39, 5, 4,
  6, 7, 15, 16, 17
};

const uint8_t relayPins[NUM_STAIRS] = {
  14, 13, 12, 11, 10,
  9, 8, 18, 38, 37,
  36, 35, 48, 47, 21
};

const uint8_t SENSOR_ACTIVE = LOW;
const uint8_t RELAY_ON  = HIGH;
const uint8_t RELAY_OFF = LOW;

// Phải giữ tín hiệu ổn định đủ lâu mới công nhận
const unsigned long SENSOR_CONFIRM_TIME = 20;

// Trạng thái thô lần trước
bool lastRawState[NUM_STAIRS];

// Trạng thái đã lọc
bool stableState[NUM_STAIRS];

// Thời điểm trạng thái bắt đầu thay đổi
unsigned long changeTime[NUM_STAIRS];


void setup() {

  for (int i = 0; i < NUM_STAIRS; i++) {
    pinMode(sensorPins[i], INPUT_PULLUP);

    lastRawState[i] = false;
    stableState[i] = false;
    changeTime[i] = 0;
  }

  for (int i = 0; i < NUM_STAIRS; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], RELAY_OFF);
  }
}


void loop() {

  unsigned long now = millis();

  bool relayRequest[NUM_STAIRS] = {false};

  // =========================
  // LỌC SENSOR
  // =========================

  for (int i = 0; i < NUM_STAIRS; i++) {

    bool rawState =
      (digitalRead(sensorPins[i]) == SENSOR_ACTIVE);

    // Nếu trạng thái thô vừa thay đổi
    if (rawState != lastRawState[i]) {
      lastRawState[i] = rawState;
      changeTime[i] = now;
    }

    // Chỉ công nhận trạng thái mới
    // nếu nó giữ ổn định đủ lâu
    if ((now - changeTime[i]) >= SENSOR_CONFIRM_TIME) {
      stableState[i] = rawState;
    }
  }


  // =========================
  // TÍNH RELAY CẦN BẬT
  // =========================

  for (int i = 0; i < NUM_STAIRS; i++) {

    if (stableState[i]) {

      if (i > 0) {
        relayRequest[i - 1] = true;
      }

      relayRequest[i] = true;

      if (i < NUM_STAIRS - 1) {
        relayRequest[i + 1] = true;
      }
    }
  }


  // =========================
  // CẬP NHẬT RELAY
  // =========================

  for (int i = 0; i < NUM_STAIRS; i++) {

    digitalWrite(
      relayPins[i],
      relayRequest[i] ? RELAY_ON : RELAY_OFF
    );
  }
}