#include <LiquidCrystal_I2C.h>
#include "DHT.h"

DHT dht(7, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int LED = 11;

void setup() {
    Serial.begin(9600);
    pinMode(LED, OUTPUT);
    lcd.init();
    lcd.background();
}

void loop() {
    
}
