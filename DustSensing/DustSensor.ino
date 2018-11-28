#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define debug true

DHT dht(7, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int LED = 11;
const int samplingTime = 280;
const int deltaTime = 40;
const int sleepTime = 9680;

float latestDust = 0;

void setup() {
    Serial.begin(9600);
    pinMode(LED, OUTPUT);
    lcd.init();
    lcd.backlight();
}

void loop() {
    digitalWrite(LED, LOW);
    delayMicroseconds(samplingTime);
    const float dust = analogRead(0);
    delayMicroseconds(deltaTime);
    digitalWrite(LED, HIGH);
    delayMicroseconds(sleepTime);
    const float voltage = dust * (5.0 / 1024.0);  // 전압 구하기
    const float dustug = 0.17 * voltage;      // ug 단위 변환
    const float dus = dustug * 1000;

    const int h = dht.readHumidity();     // 습도 값 구하기
    const int t = dht.readTemperature();  // 온도 값 구하기

    lcd.setCursor(0, 0);
    lcd.print("H: "); lcd.print(h); lcd.print(" %  ");
    lcd.print("T: "); lcd.print(t); lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Dust: ");

    if (dus > 0) latestDust = dus;
    lcd.print(latestDust);
    lcd.print("ug");

    if (debug) {
        Serial.print("Dust Sensor Voltage: ");
        Serial.print(voltage);
        Serial.println("V");
        Serial.print("Dust Value: ");
        Serial.print(dus); // unit: mg/m3
        Serial.println("ug");
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.println(" %\t");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" C");
    }
}
