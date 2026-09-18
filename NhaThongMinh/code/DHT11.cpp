/* #include <Arduino.h>
#include <DHT.h>

//================= KHAI BÁO =================//

#define DHTTYPE DHT11

#define DHT11_SENSOR1 25
#define DHT11_SENSOR2 33
#define DHT11_SENSOR3 32

DHT dht1(DHT11_SENSOR1, DHTTYPE);
DHT dht2(DHT11_SENSOR2, DHTTYPE);
DHT dht3(DHT11_SENSOR3, DHTTYPE);

//================= SETUP =================//

void setup()
{
    Serial.begin(115200);

    dht1.begin();
    dht2.begin();
    dht3.begin();

    Serial.println("===== TEST 3 CAM BIEN DHT11 =====");
}

//================= LOOP =================//

void loop()
{
    float temp1 = dht1.readTemperature();
    float hum1  = dht1.readHumidity();

    float temp2 = dht2.readTemperature();
    float hum2  = dht2.readHumidity();

    float temp3 = dht3.readTemperature();
    float hum3  = dht3.readHumidity();

    Serial.println("--------------------------------");

    if (isnan(temp1) || isnan(hum1))
    {
        Serial.println("DHT1: Loi doc du lieu!");
    }
    else
    {
        Serial.print("DHT1 -> Nhiet do: ");
        Serial.print(temp1);
        Serial.print(" °C | Do am: ");
        Serial.print(hum1);
        Serial.println(" %");
    }

    if (isnan(temp2) || isnan(hum2))
    {
        Serial.println("DHT2: Loi doc du lieu!");
    }
    else
    {
        Serial.print("DHT2 -> Nhiet do: ");
        Serial.print(temp2);
        Serial.print(" °C | Do am: ");
        Serial.print(hum2);
        Serial.println(" %");
    }

    if (isnan(temp3) || isnan(hum3))
    {
        Serial.println("DHT3: Loi doc du lieu!");
    }
    else
    {
        Serial.print("DHT3 -> Nhiet do: ");
        Serial.print(temp3);
        Serial.print(" °C | Do am: ");
        Serial.print(hum3);
        Serial.println(" %");
    }

    Serial.println();

    delay(2000); // DHT11 cần khoảng 2 giây giữa các lần đọc
} */