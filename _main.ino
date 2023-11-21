#include <DHT.h>
#include <MQ2.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>

// Pin Definitions
const int trigPin = 10;
const int echoPin = 9;
const int buzzerPin = 7;
const int DHTPIN = 7;
const int smokeSensorPin = A0;

// Sensor Objects
DHT dht(DHTPIN, DHT11);
MQ2 mq2(smokeSensorPin);

// LCD Display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// GSM Module
SoftwareSerial gsm(8, 6); // RX, TX for the GSM module

float duration, distance, previousDistance;
unsigned long previousMillis = 0;
unsigned long interval = 1000; // Set the interval for speed calculation in milliseconds
int h, t, lpg, co, smoke;

void setup() {
  Serial.begin(9600);
  dht.begin();
  mq2.begin();
  lcd.init();
  lcd.backlight();
  pinMode(buzzerPin, OUTPUT);
  gsm.begin(9600);
  delay(1000);
  gsm.println("AT");
  delay(1000);
  gsm.println("AT+CMGF=1"); // Set SMS mode to text
  delay(1000);
}

void loop() {
  // Read DHT11 temperature and humidity
  h = dht.readHumidity();
  t = dht.readTemperature();

  // Display DHT11 data and send SMS for extreme temperature
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %, Temp: ");
  Serial.print(t);
  Serial.println(" °C");
  lcd.setCursor(0, 0);
  lcd.print(" Temperature ");
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(t);
  lcd.print("C");
  lcd.setCursor(11, 1);
  lcd.print("H:");
  lcd.print(h);
  lcd.print("%");

  if (t < 27) {
    lcd.setCursor(0, 2);
    lcd.print("Condition: Room Temp");
    noTone(buzzerPin);
  } else if (t <= 40) {
    lcd.setCursor(0, 2);
    lcd.print("Condition: High Temp");
    noTone(buzzerPin);
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Condition: Extreme Temp");
    activateBuzzer();
    sendSMS("+254791065938", "Extreme Temperature Conditions Detected");
  }

  // Read smoke sensor and send SMS for smoke detection
  smoke = mq2.readSmoke();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smoke Sensor");
  lcd.setCursor(0, 1);
  lcd.print("Smoke Level: ");
  lcd.print(smoke);
  lcd.setCursor(0, 2);
  lcd.print("LPG Level: ");
  lcd.print(lpg);
  lcd.setCursor(0, 3);
  lcd.print("CO Level: ");
  lcd.print(co);

  if (smoke > 1000) {
    activateBuzzer();
    sendSMS("+254791065938", "Smoke Presence Detected. Please Check Tank");
  } else {
    noTone(buzzerPin);
  }

  // Ultrasonic sensor to monitor tank level
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.0343) / 2;

   // Calculate speed of change in distance over time
  unsigned long currentMillis = millis();
  float speed = (distance - previousDistance) / ((currentMillis - previousMillis) / 1000.0);
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Speed: ");
  Serial.print(speed);
  Serial.println(" cm/s ");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print(" cm ");

  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(speed);
  lcd.print(" cm/s ");

  lcd.setCursor(0, 2);
  lcd.print("Available: ");
  lcd.print(30 - distance);
  lcd.print(" L ");

  if (distance > 7 && distance <= 13) {
    sendSMS("+254791065938", "Petroleum Tank Level Below Threshold: " + String(distance) + " cm");
  } else if (distance > 13) {
    sendSMS("+254791065938", "Petroleum Tank Depleted");
    makeCall();
  }

  if (distance >= 10) {
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
  }

  delay(1000);
}

void activateBuzzer() {
  tone(buzzerPin, 1000); // Continuous buzzer sound
}

void sendSMS(String phoneNumber, String message) {
  gsm.println("AT+CMGS=\"" + phoneNumber + "\"");
  delay(1000);
  gsm.print(message);
  gsm.write(26);
  delay(1000);
}

void makeCall() {
  gsm.println("ATD+254791065938;"); // Replace with the recipient's phone number
  delay(1000);
}
