/* #include <Arduino.h>

float Error = 0, LastError = 0;

float Kp = 30, Kd = 8;                       // Declare PID Parameters (Readjust)

int SpeedBase = 50;                                                  // 1023 Declare Speed

const int SensorPin[8] = {36, 39, 34, 35, 32, 33, 25, 26};           // Declare Sensor LDR
int Black[8], White[8];

void Calibration(){

    for(int i = 0; i < 8; i++) Black[i] = 0, White[i] = 4095;

    unsigned long Start = millis();

    while (millis() - Start < 5000){
        for(int i = 0; i < 8; i++){
            int val = analogRead(SensorPin[i]);

            if(val > Black[i]) Black[i] = val;  // Find Max
            if(val < White[i]) White[i] = val;  // Find Min
        }   delay(5);
    }
}

void setup(){
    pinMode(19, OUTPUT);      
    pinMode(27, OUTPUT);

    ledcSetup(0, 1000, 10);   
    ledcAttachPin(19, 0);                  // Create a channel (PWM), Attach Channel -> Motor         
    
    ledcSetup(1, 1000, 10);   
    ledcAttachPin(27, 1);

    Calibration();   

    Serial.begin(115200);   

    for(int i = 0; i < 8; i++){
        Serial.print("W"); Serial.print(i); Serial.print(": "); Serial.print(White[i]);
        Serial.print("  B"); Serial.print(i); Serial.print(": "); Serial.println(Black[i]);
    }
}

float SetError() {
    float average = 0;
    float sum = 0;

    const float weight[8] = {-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5};

    for (int i = 0; i < 8; i++) {
        int raw = analogRead(SensorPin[i]);

        
        int val = constrain(raw, White[i], Black[i]);

        int diff = Black[i] - White[i];
        if (diff <= 0) diff = 1;   

        
        float value = (float)(val - White[i]) * 1000.0 / diff;

        
        if (value < 0) value = 0;
        if (value > 1000) value = 1000;

        
        if (value < 50) value = 0;

        average += value * weight[i];
        sum += value;
    }

    
    if (sum < 1) {
        return LastError;  
    }

    float error = average / sum;

    
    if (error > 3.5) error = 3.5;
    if (error < -3.5) error = -3.5;

    LastError = error;   // cập nhật lại

    return error;
}

void loop(){
    Error = SetError();
    float PID = constrain(Kp * Error + Kd * (Error - LastError), -700, 700);
    LastError = Error;

    int LeftSpeed  = constrain(SpeedBase + PID, 0, 1023);
    int RightSpeed = constrain(SpeedBase - PID, 0, 1023);

    ledcWrite(0, RightSpeed);    
    ledcWrite(1, LeftSpeed);   
    delay(1);
}

*/

#include <Arduino.h>

// ===================== MOTOR CONFIG =====================
const int LEFT_MOTOR_PIN  = 27;
const int RIGHT_MOTOR_PIN = 19;

const int LEFT_CHANNEL  = 1;
const int RIGHT_CHANNEL = 0;

const int PWM_FREQ = 500;     // PC817 chậm, nên để thấp
const int PWM_RES  = 10;      // 0..1023
const int PWM_MAX  = 1023;

// ===================== SENSOR CONFIG =====================
const int SENSOR_PIN[8] = {36, 39, 34, 35, 32, 33, 25, 26};
const float WEIGHT[8]   = {-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5};

int sensorBlack[8];
int sensorWhite[8];
int sensorValue[8];

// ===================== PID CONFIG =====================
float error = 0.0f;
float lastError = 0.0f;
float integral = 0.0f;

float Kp = 25.0f;
float Ki = 0.0f;
float Kd = 5.0f;

int speedBase = 90;

// ===================== CALIBRATION =====================
void calibration() {
  for (int i = 0; i < 8; i++) {
    sensorBlack[i] = 0;
    sensorWhite[i] = 4095;
  }

  unsigned long start = millis();

  while (millis() - start < 5000) {
    for (int i = 0; i < 8; i++) {
      int raw = analogRead(SENSOR_PIN[i]);

      if (raw > sensorBlack[i]) sensorBlack[i] = raw;
      if (raw < sensorWhite[i]) sensorWhite[i] = raw;
    }
    delay(5);
  }
}

// ===================== SENSOR READ =====================
float readLineError() {
  float weightedSum = 0.0f;
  float signalSum = 0.0f;

  for (int i = 0; i < 8; i++) {
    int raw = analogRead(SENSOR_PIN[i]);

    int minVal = sensorWhite[i];
    int maxVal = sensorBlack[i];
    if (maxVal <= minVal) maxVal = minVal + 1;

    raw = constrain(raw, minVal, maxVal);

    float value = (float)(raw - minVal) * 1000.0f / (float)(maxVal - minVal);

    if (value < 50.0f) value = 0.0f;
    if (value > 1000.0f) value = 1000.0f;

    sensorValue[i] = (int)value;

    weightedSum += value * WEIGHT[i];
    signalSum += value;
  }

  if (signalSum < 1.0f) {
    return lastError;
  }

  float pos = weightedSum / signalSum;

  if (pos > 3.5f) pos = 3.5f;
  if (pos < -3.5f) pos = -3.5f;

  return pos;
}

// ===================== MOTOR CONTROL =====================
void setMotor(int leftSpeed, int rightSpeed) {
  leftSpeed  = constrain(leftSpeed, 0, PWM_MAX);
  rightSpeed = constrain(rightSpeed, 0, PWM_MAX);

  ledcWrite(LEFT_CHANNEL, leftSpeed);
  ledcWrite(RIGHT_CHANNEL, rightSpeed);
}

// ===================== DEBUG =====================
void printDebug(float pid, int leftSpeed, int rightSpeed) {
  for (int i = 0; i < 8; i++) {
    Serial.print(sensorValue[i]);
    Serial.print('\t');
  }

  Serial.print("| err=");
  Serial.print(error, 3);
  Serial.print(" pid=");
  Serial.print(pid, 1);
  Serial.print(" L=");
  Serial.print(leftSpeed);
  Serial.print(" R=");
  Serial.println(rightSpeed);
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(LEFT_MOTOR_PIN, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN, OUTPUT);

  ledcSetup(LEFT_CHANNEL, PWM_FREQ, PWM_RES);
  ledcSetup(RIGHT_CHANNEL, PWM_FREQ, PWM_RES);

  ledcAttachPin(LEFT_MOTOR_PIN, LEFT_CHANNEL);
  ledcAttachPin(RIGHT_MOTOR_PIN, RIGHT_CHANNEL);

  calibration();

  for (int i = 0; i < 8; i++) {
    Serial.print("W"); Serial.print(i); Serial.print(": "); Serial.print(sensorWhite[i]);
    Serial.print("  B"); Serial.print(i); Serial.print(": "); Serial.println(sensorBlack[i]);
  }
}

// ===================== LOOP =====================
void loop() {
  error = readLineError();

  float derivative = error - lastError;
  integral += error;
  integral = constrain(integral, -50.0f, 50.0f);

  float pid = Kp * error + Ki * integral + Kd * derivative;
  pid = constrain(pid, -400.0f, 400.0f);

  int leftSpeed  = speedBase + (int)pid;
  int rightSpeed = speedBase - (int)pid;

  leftSpeed  = constrain(leftSpeed, 0, PWM_MAX);
  rightSpeed = constrain(rightSpeed, 0, PWM_MAX);

  setMotor(leftSpeed, rightSpeed);
  printDebug(pid, leftSpeed, rightSpeed);

  lastError = error;
  delay(5);
}

