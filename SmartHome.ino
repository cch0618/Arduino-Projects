#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const int pinDHT = 3; // 온도센서 핀을 3번으로 정의합니다.
const int pinPiezo = 2; // 피에조 부저 핀을 2번으로 정의합니다.
const int pinAirConditioner = 7; // 에어컨 LED 핀을 7번으로 정의합니다.
const int pinCDS = A0; // 조도센서 핀을 Analog 0번으로 정의합니다.
DHT_Unified dht(3, DHT11); // 온도센서를 정의합니다.

LiquidCrystal_I2C TV(0x27, 16, 2); // TV를 정의합니다.
SoftwareSerial bluetooth(4, 5); // 블루투스
Servo blind; // 블라인드를 움직이는 서보모터입니다.

// 에어컨이 켜질 때 나오는 음악
int notes[] = {261, 294, 330, 349, 392, 440, 494, 523};
int brightness = 0; // 밝기의 기본값입니다.

float humidity = 0.; // 습도의 기본값입니다.
float temperature = 0.; //온도의 기본값입니다.
float discomfortIndex = 0.; //불쾌지수의 기본값입니다.

boolean isAirConditionerTurnOn = false; // 에어컨이 켜져 있는지
boolean isBlindOpen = false; // 블라인드가 열렸는지
// boolean isBlindCloseByUser = false;
boolean isTVTurnOn = false; // TV가 켜져있는지
boolean isLightTurnOffByUser = false; // 전등이 블루투스 조작에 의해 꺼졌는지

boolean isTVModePrintTemperature = false; // TV에서 온도를 출력해야 하는지

// 아두이노가 켜질 때 한번만 실행되는 함수입니다.
void setup() {
  pinMode(pinPiezo, OUTPUT);
  blind.attach(6); // 서보모터를 6번 핀에 연결
  dht.begin();

  // 7번부터 13번까지의 LED 핀의 모드를 출력으로 변경
  for (int i = 7; i <= 13; i++) {
    pinMode(i, OUTPUT);
  }

  TV.init(); // TV 초기화
  Serial.begin(9600);
  bluetooth.begin(9600);
}

// TV를 켜는 함수입니다.
void turnOnTV(String content = "Hello, World!") {
  TV.setCursor(0, 0);
  TV.backlight();
  TV.print(content); // TV에 출력
  isTVTurnOn = true;
}

void turnOnTV(String temp, String humi) {
  TV.setCursor(0, 0);
  TV.backlight();
  TV.print(temp); // TV에 출력
  TV.setCursor(0, 1);
  TV.print(humi); // TV에 출력
  isTVTurnOn = true;
}

// TV를 끄는 함수입니다.
void turnOffTV() {
  TV.clear();
  TV.noBacklight();
  isTVTurnOn = false;
}

// 에이컨을 켜는 함수입니다.
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

// 에어컨을 끄는 함수입니다.
void turnOffAirConditioner() {
  digitalWrite(pinAirConditioner, LOW);
  isAirConditionerTurnOn = false;
}

// 전등을 켜는 함수입니다.
void turnOnLights() {
  //FIXME: 3색 LED 2개 각각의 RGB에 HIGH를 넘겨주면 색이 다름.
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  //digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  //digitalWrite(13, HIGH);
}

// 전등을 끄는 함수입니다.
void turnOffLights() {
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
}

// 블라인드를 내리는 함수입니다.
void closeBlind() {
  blind.attach(6); blind.write(180);
  isBlindOpen = false;
}

// 블라인드를 올리는 함수입니다.
void openBlind() {
  blind.attach(6); blind.write(80);
  isBlindOpen = true;
}

// 아두이노가 켜져 있는 동안 계속 실행되는 함수입니다.
void loop() {
  // 온도센서로 온도와 습도 측정
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  humidity = event.relative_humidity;
  temperature = event.temperature;
  // 불쾌지수 계산
  discomfortIndex = (1.8 * temperature) - (0.55 * (1 - humidity / 100.0) * (1.8 * temperature - 26)) + 32;
  // 불쾌지수가 73 이상일 경우 에어컨 작동
  if (discomfortIndex >= 73) turnOnAirConditioner();
  else turnOffAirConditioner();

  // 조도센서로 밝기 측정
  brightness = analogRead(pinCDS);
  // 어두우면 블라인드를 내리고 모든 전등을 킴
  if (brightness < 600) {
    if (isBlindOpen) closeBlind();
    // 전등이 꺼지지 않은 상태에서만 킴
    if (!isLightTurnOffByUser) turnOnLights();
  }
  // 밝으면 블라인드를 올리고 모든 전등을 끔
  else {
    if (!isBlindOpen) openBlind();
    turnOffLights();
  }

  // 블루투스가 사용 가능할 경우에만 실행합니다.
  if (bluetooth.available()) {
    char command = bluetooth.read(); // 블루투스로 전송된 명령을 변수에 저장합니다.

    // 명령이 1일 경우 TV를 켭니다.
    if (command == '1') {
      if (isTVTurnOn) turnOffTV();
      else {
          if (!isTVModePrintTemperature) turnOnTV();
          // TV의 모드가 온습도 출력 모드이면 온습도를 출력합니다.
          else turnOnTV(String("temperature:" + String(temperature)), String("humidity:" + String(humidity)));
      }
    }
    // 명령이 2일 경우 전등의 상태를 조절합니다.
    else if (command == '2') {
        isLightTurnOffByUser = !isLightTurnOffByUser;
    }
    // 명령이 3일 경우 TV의 모드를 조정합니다.
    else if (command == '3') {
        isTVModePrintTemperature = !isTVModePrintTemperature;
    }
  }
}
