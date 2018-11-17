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
boolean isAirConditionerTurnOffByUser = false;
boolean isBlindOpen = false;
boolean isBlindCloseByUser = false;
boolean isTVTurnOn = false;


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


void turnOnTV(String content = "Hello, World!") {
  TV.setCursor(0, 0);
  TV.backlight();
  TV.print(content);
  isTVTurnOn = true;
}

void turnOffTV() {
  TV.clear();
  TV.noBacklight();
  isTVTurnOn = false;
}

void turnOnAirConditioner() {
  digitalWrite(pinAirConditioner, HIGH);
  // 에어컨이 꺼져있는 상태였을 시 음악 재생
  if (!isAirConditionerTurnOn) {
    for (int i = 0; i < 8; i++) {
      tone(pinPiezo, notes[i], 300); delay(100);
    }
    isAirConditionerTurnOn = true;
  }
}

void turnOffAirConditioner() {
  digitalWrite(pinAirConditioner, LOW);
  isAirConditionerTurnOn = false;
}

void turnOnLights() {
  //FIXME: 3색 LED 2개 각각의 RGB에 HIGH를 넘겨주면 색이 다름.
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  //digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  //digitalWrite(13, HIGH);
}

void turnOffLights() {
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
}

void closeBlind() {
  blind.attach(6); blind.write(180);
  isBlindOpen = false;
}

void openBlind() {
  blind.attach(6); blind.write(80);
  isBlindOpen = true;
}


void loop() {
  // 온도센서로 온도와 습도 측정
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  humidity = event.relative_humidity;
  temperature = event.temperature;
  // 불쾌지수 계산
  discomfortIndex = (1.8 * temperature) - (0.55 * (1 - humidity / 100.0) * (1.8 * temperature - 26)) + 32;
  Serial.println(discomfortIndex);
  // 불쾌지수가 73 이상일 경우 에어컨 작동
  if (discomfortIndex >= 73 && !isAirConditionerTurnOffByUser) turnOnAirConditioner();
  else {
    if (isAirConditionerTurnOffByUser) turnOffAirConditioner();
  }

  // 조도센서로 밝기 측정
  brightness = analogRead(pinCDS);
  // 어두우면 블라인드를 내리고 모든 전등을 킴
  if (brightness < 600) {
    if (isBlindOpen) closeBlind();
    turnOnLights();
  } else {
    if (!isBlindOpen) openBlind();
    turnOffLights();
  }

  if (bluetooth.available()) {
    char command = bluetooth.read();

    if (command == '1') {
      if (isTVTurnOn) turnOffTV();
      else turnOnTV();
    }
    else if (command == '2') {
      turnOnAirConditioner();
      isAirConditionerTurnOffByUser = false;
    }
    else if (command == '3') {
      turnOffAirConditioner();
      isAirConditionerTurnOffByUser = true;
    }
  }
}
