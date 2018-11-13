#include <SoftwareSerial.h>
#include <Servo.h>  // 서보모터를 사용하기 위한 라이브러리 호출하기
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // I2C LCD를 사용하기 위한 라이브러리 호출하기
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


// I2C LCD에 출력할 아이콘 생성
byte temp[] = {
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};

byte humi[] = {
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B11111,
  B11111,
  B01110
};

byte C[] = {
  B10000,
  B00110,
  B01001,
  B01000,
  B01000,
  B01001,
  B00110,
  B00000
};

int DHTPin = 3;   // 온습도센서를 오렌지보드 3번핀에 연결

DHT_Unified dht(3, DHT11);

uint32_t delayMS;

LiquidCrystal_I2C TV(0x27, 16, 2);  // I2C LCD를 TV라는 이름으로 선언
SoftwareSerial BTSerial(4, 5);  // 블루투스를 사용하기 위해 BTSerial 이름으로 선언
Servo blind;  // 서보모터를 blind라는 이름으로 선언

// 현재 LED와 LCD의 상태를 저장하는 변수 선언
boolean ledR = false;
boolean ledG = false;
boolean ledB = false;
boolean led1R = false;
boolean led1G = false;
boolean led1B = false;
boolean ledGAS = false;
boolean blindOpen = false;
boolean LCD1 = false;
boolean LCD2 = false;

boolean flag = 0;

// 현재 온습도 값을 저장하는 변수 선언
byte temperature = 0;
byte humidity = 0;

//
int notes[] = {4186, 6271};
int tempo = 200;
int airCon = 7;  // RGB LED의 파란색 핀을 디지털 9번 연결
int piezoPin = 2; // 피에조 부저의 +극을 오렌지보드 2번에 연결

void setup() {
  pinMode(2, OUTPUT);  // 피에조 핀을 출력핀으로 설정
  blind.attach(6); // 서보모터를 6번핀에 연결
  dht.begin();

  // 모든 LED 핀을 출력핀으로 설정
  for (int i = 7; i <= 13; i++) {
    pinMode(i, OUTPUT);
  }

  TV.init();  // I2C LCD의 기본 설정
  Serial.begin(9600);  // 시리얼 통신을 위해 시리얼 통신속도를 9600으로 설정
  BTSerial.begin(9600);  // 블루투스 통신을 위해 블루투스 통신속도를 9600으로 설정

  // 위에서 만든 I2C LCD에 출력할 아이콘을 메모리에 저장(0, 1, 2번 메모리)
  TV.createChar((byte)0, temp);
  TV.createChar(1, humi);
  TV.createChar(2, C);
}


void loop() {
  // 조도 센서 값 측정
  int cdsVal = analogRead(A0);
  Serial.println(cdsVal);

  // 블루투스 통신을 통해 데이터가 들어오면
  if (BTSerial.available()) {
    // c라는 변수에 블루투스를 통해 전달받은 데이터를 넣어줍니다.(문자 1개)
    char c = BTSerial.read();

    // 전달받은 데이터에 따라 그에 맞는 동작을 해줍니다.
    if (c == 'a')
      setLED(8, ledR);

    else if (c == 'b')
      setLED(9, ledG);

    else if (c == 'c')
      setLED(10, ledB);

    else if (c == 'd')
      setLED(7, ledGAS);

    else  if (c == 'e') {
      if (!LCD1) {
        TV.backlight();
      }
      else {
        TV.noBacklight();
        TV.clear();
      }
      LCD1 = !LCD1;
    }
    else if (c == 'f') {
      TV.clear();
      LCD2 = !LCD2;
    }
    else  if (c == 'g') {
      setLED(11, led1R);
    }
    else if (c == 'h') {
      setLED(12, led1G);
    }
    else if (c == 'i') {
      setLED(13, led1B);
    }
  }

  // 온습도센서로 온도와 습도값 측정(잘못 연결됬을 경우 failed 메세지 출력)
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  float humidity = event.relative_humidity;
  float temperature = event.temperature;
  float discomfortIndex = (1.8 * temperature) - (0.55 * (1 - humidity / 100.0) * (1.8 * temperature - 26)) + 32;
  // 불쾌지수값이 72보다 높을 경우
  if (discomfortIndex > 72) {
    // 에어콘(파란색 LED)를 켜주면서 멜로디를 재생
    digitalWrite(airCon, HIGH);
    if (flag == 0) {
      for (int i = 0; i < 5; i++) {
        tone(piezoPin, notes[i], 300);
        delay(150);
      }
      flag = 1;
    }
  }

  else {
    // 에어콘(파란색 LED)를 끔
    digitalWrite(airCon, LOW);
    flag = 0;
  }
  
  // I2C LCD가 온습도 출력 모드면 현재 온도와 습도를 I2C LCD에 출력
  if (LCD2) {
    TV.setCursor(0, 0);
    TV.print("Today's weather");

    TV.setCursor(0, 1);
    TV.write((byte)0);
    TV.print(" ");
    TV.print(event.temperature);
    TV.write(2);

    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println("Error reading humidity!");
    }
    TV.setCursor(6, 1);
    TV.write(1);
    TV.print(" ");
    TV.print(event.relative_humidity);
    TV.print("%");
    delay(100);
  }
  else {  // I2C LCD가 조도 출력 모드면 조도 값에 따라 현재 블라인드 동작을 I2C에 출력
    TV.clear();
    if (cdsVal > 200) {
      TV.print("blind open!");
    }
    else {
      TV.print("blind close");
    }
    delay(100);
  }

  // 조도 센서로 측정된 값이 어두우면 블라인드를 올리고 경보음을 냅니다.
  if (cdsVal < 600 && !blindOpen) {
    blind.attach(6);
    TV.setCursor(0, 0);
    blind.write(80);
    for (int i = 0; i < 2; i++) {
      tone(2, notes[i], tempo);
      delay(100);
    }
    blindOpen = !blindOpen;
    blind.detach();
  }
  else if (cdsVal > 900 && blindOpen) { // 조도 센서로 측정된 값이 밝으면 블라인드를 내립니다.
    blind.attach(6);
    TV.setCursor(0, 0);
    blind.write(180);
    blindOpen = !blindOpen;
    delay(200);
    blind.detach();
  }
}


// LED의 현재 상태에 따라 LED를 키거나 꺼줍니다.
void setLED(int PIN, boolean &LED) {
  if (!LED) {
    digitalWrite(PIN, HIGH);
  }
  else {
    digitalWrite(PIN, LOW);
  }
  LED = !LED;
}
