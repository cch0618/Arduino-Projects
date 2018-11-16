#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const int pinDHT = 3; // 온도센서
const int pinPiezo = 2; // 피에조 부저
const int pinAirConditioner = 7; // 에어컨 LED
const int pinCDS = A0; // 조도센서
DHT_Unified dht(3, DHT11);

LiquidCrystal_I2C TV(0x27, 16, 2); // TV
SoftwareSerial bluetooth(4, 5); // 블루투스
Servo blind; // 블라인드를 움직이는 서보모터

// 에어컨이 켜질 때 나오는 음악
int notes[] = {261, 294, 330, 349, 392, 440, 494, 523};
int brightness = 0; // 밝기

float humidity = 0.; // 습도
float temperature = 0.; //온도
float discomfortIndex = 0.; //불쾌지수

boolean isAirConditionerTurnOn = false;
boolean isBlindOpen = false;


void setup() {
  pinMode(pinPiezo, OUTPUT);
  blind.attach(6); // 서보모터를 6번 핀에 연결
  dht.begin();

  // 7번부터 13번까지의 LED 핀의 모드를 출력으로 변경
  for (int i = 7; i <= 13; i++) {
    pinMode(i, OUTPUT);
  }

  TV.init();
  Serial.begin(9600);
  bluetooth.begin(9600);
}


void loop() {
  // 온도센서로 온도와 습도 측정
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  humidity = event.relative_humidity;
  temperature = event.temperature;
  // 불쾌지수 계산
  discomfortIndex = (1.8 * temperature) - (0.55 * (1 - humidity / 100.0) * (1.8 * temperature - 26)) + 32;
  // 불쾌지수가 74 이상일 경우 에어컨 LED 작동
  if (discomfortIndex >= 74) {
    digitalWrite(pinAirConditioner, HIGH);
    // 에어컨이 꺼져있는 상태였을 시 음악 재생
    if (!isAirConditionerTurnOn) {
      for (int i = 0; i < 8; i++) {
        tone(pinPiezo, notes[i], 300); delay(100);
      }
      isAirConditionerTurnOn = true;
    }
  } else {
    digitalWrite(pinAirConditioner, LOW);
    isAirConditionerTurnOn = false;
  }

  // 조도센서로 밝기 측정
  brightness = analogRead(pinCDS);
  // 어두우면 블라인드를 내리고 전등을 킴
  if (brightness < 600) {
    if (isBlindOpen) {
      blind.attach(6); blind.write(180);
      isBlindOpen = false;
    }
    digitalWrite(11, HIGH);
  } else {
    if (!isBlindOpen) {
      blind.attach(6); blind.write(80);
      isBlindOpen = true;
    }
    digitalWrite(11, LOW);
  }
}
