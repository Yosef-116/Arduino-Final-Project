#include <IRremote.h>
#include <Wire.h>
#include <RTClib.h>

const int IR_RECEIVE_PIN = 7;

// Motor driver pins
const int motorPin1 = 8;
const int motorPin2 = 9;

// LED pin
const int ledPin = 10;

// Flame sensor and buzzer
const int flamePin = 11;
const int buzzerPin = 12;

// RTC object
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(flamePin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("IR + Fan + Light + RTC + Flame Sensor Ready...");
}

void loop() {
  static unsigned long lastTimePrint = 0;
  static DateTime now;

  // Show time every second
  if (millis() - lastTimePrint > 1000) {
    now = rtc.now();
    Serial.print("Time: ");
    Serial.print(now.hour());
    Serial.print(':');
    Serial.print(now.minute());
    Serial.print(':');
    Serial.println(now.second());
    lastTimePrint = millis();
  }

  // Auto Fan Control Based on Time
  if (now.hour() == 19 && now.minute() == 21 && now.second() == 15) {
    Serial.println("⏰ Auto Fan ON (19:21:15)");
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
  }

  if (now.hour() == 19 && now.minute() == 22 && now.second() == 0) {
    Serial.println("⏰ Auto Fan OFF (19:22:00)");
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
  }

  // 🔥 Flame detection and buzzer (only for flame!)
  int flameDetected = digitalRead(flamePin);
  if (flameDetected == HIGH) {
    Serial.println("🔥 Flame Detected! Sounding Alarm for 5 seconds...");
    digitalWrite(buzzerPin, HIGH);
    delay(5000);
    digitalWrite(buzzerPin, LOW);
  }

  // 📟 IR Remote control
  if (IrReceiver.decode()) {
    uint8_t code = IrReceiver.decodedIRData.command;
    Serial.print("Received IR Code: ");
    Serial.println(code);

    switch (code) {
      case 7: // Fan ON
        Serial.println("Fan ON (Manual)");
        digitalWrite(motorPin1, HIGH);
        digitalWrite(motorPin2, LOW);
        break;

      case 21: // Fan OFF
        Serial.println("Fan OFF (Manual)");
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, LOW);
        break;

      case 68: // LED ON
        Serial.println("Light ON");
        digitalWrite(ledPin, HIGH);
        break;

      case 64: // LED OFF
        Serial.println("Light OFF");
        digitalWrite(ledPin, LOW);
        break;

      default:
        Serial.println("Unknown button");
        break;
    }

    IrReceiver.resume(); // Ready for next IR input
  }
}
