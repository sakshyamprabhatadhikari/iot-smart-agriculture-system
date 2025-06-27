#define BLYNK_TEMPLATE_ID "TMPL6s-n81pG1"         // Blynk Template ID
#define BLYNK_TEMPLATE_NAME "Watering"            // Blynk Template Name

//including the necessary library
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Blynk and WiFi credentials
char auth[] = "fJ3FG6-KuN0jrT7qT3FlbYGhtRwgaAwQ"; // Blynk auth token
char ssid[] = "ALHN-2476";                      // WiFi SSID
char pass[] = "a753gsDDVV";                       // WiFi password

// Pin definitions
#define DHTPIN 4          // DHT11 data pin
#define DHTTYPE DHT11     // DHT11 sensor type
#define SOIL_PIN 32      // Soil moisture sensor analog pin
#define TRIG_PIN 12       // Ultrasonic trigger pin
#define ECHO_PIN 34       // Ultrasonic echo pin
#define BUZZER_PIN 19     // Buzzer pin
#define RELAY_PIN 27      // Relay control pin

// Thresholds
const int DRY_THRESHOLD = 40;     // Soil moisture below 40% is dry
const int LOW_WATER_LEVEL = 5;    // Water level below 5cm is low

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() {
  Serial.begin(115200);           // Start serial communication
  pinMode(TRIG_PIN, OUTPUT);      // Set ultrasonic trigger pin
  pinMode(ECHO_PIN, INPUT);       // Set ultrasonic echo pin
  pinMode(BUZZER_PIN, OUTPUT);    // Set buzzer pin
  pinMode(RELAY_PIN, OUTPUT);     // Set relay pin
  digitalWrite(RELAY_PIN, LOW);   // Initialize relay off
  dht.begin();                    // Initialize DHT11 sensor
  lcd.init();                     // Initialize LCD
  lcd.backlight();                // Enable LCD backlight
  lcd.setCursor(0, 0);            // Set cursor to top-left
  lcd.print("Starting...");       // Display startup message
  delay(2000);                    // Wait 2 seconds
  lcd.clear();                    // Clear LCD

  Blynk.begin(auth, ssid, pass);  // Connect to Blynk server
}

void loop() {
  Blynk.run();                    // Handle Blynk communication

  // Read sensor data
  float temp = dht.readTemperature(); // Read temperature (°C)
  float hum = dht.readHumidity();     // Read humidity (%)
  int soilMoisture = map(analogRead(SOIL_PIN), 4095, 0, 0, 100); // Map soil moisture to 0-100%

  // Read water level with ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  int waterDistance = duration * 0.034 / 2; // Calculate distance (cm)
  int waterLevel = 20 - waterDistance;      // Calculate water level (tank height 20cm)

  // Control relay based on soil moisture and water level
  if (soilMoisture < DRY_THRESHOLD && waterLevel >= LOW_WATER_LEVEL) {
    digitalWrite(RELAY_PIN, LOW); // Turn on relay
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Turn off relay
  }

  // Update LCD
  lcd.clear();
  if (waterLevel < LOW_WATER_LEVEL) {
    lcd.setCursor(0, 0);
    lcd.print("Low Water: ");
    lcd.print(waterLevel);
    lcd.print("cm");
    tone(BUZZER_PIN, 1000, 500); // Buzz at 1kHz for 500ms
  } else {
    noTone(BUZZER_PIN); // Stop buzzer
    if (isnan(temp) || isnan(hum)) {
      lcd.setCursor(0, 0);
      lcd.print("DHT Error");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(temp, 1);
      lcd.print("C H:");
      lcd.print(hum, 0);
      lcd.print("%");
    }
    lcd.setCursor(0, 1);
    lcd.print("Soil: ");
    lcd.print(soilMoisture);
    lcd.print("%");
  }

  // Send data to Blynk
  if (!isnan(temp) && !isnan(hum)) {
    Blynk.virtualWrite(V0, temp);         // Send temperature to V0
    Blynk.virtualWrite(V1, hum);          // Send humidity to V1
  }
  Blynk.virtualWrite(V2, soilMoisture);   // Send soil moisture to V2
  Blynk.virtualWrite(V3, waterLevel);     // Send water level to V3

  // Log to serial monitor
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("C, Hum: ");
  Serial.print(hum);
  Serial.print("%, Soil: ");
  Serial.print(soilMoisture);
  Serial.print("%, Water: ");
  Serial.print(waterLevel);
  Serial.println("cm");

  delay(2000); // Update every 2 seconds 
}
