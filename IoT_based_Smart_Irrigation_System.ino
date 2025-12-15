#define BLYNK_TEMPLATE_ID "TMPL3nF0tQWpX"
#define BLYNK_TEMPLATE_NAME "Smart Plant Monitoring System"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// ---------------- Blynk Credentials ----------------
char auth[] = "JW5Oq7BqSgLnk-jJZaEmfeI6d7OYdLJs";
char ssid[] = "abcd";
char pass[] = "12345678";

// ---------------- Pin Configuration ----------------
#define SOIL_PIN   A0
#define PIR_PIN    D5
#define DHT_PIN    D2     // safer pin than D4
#define RELAY_PIN  D3     // Active LOW relay

// ---------------- Sensors ----------------
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

BlynkTimer timer;

// ---------------- Variables ----------------
int relayState = HIGH;   // Motor OFF (Active LOW)
int pirToggle = 0;

// ---------------- Virtual Pins ----------------
#define VPIN_TEMP        V0
#define VPIN_HUMIDITY    V1
#define VPIN_SOIL        V3
#define VPIN_PIR_LED     V5
#define VPIN_PIR_TOGGLE  V6

// ---------------- Function Prototypes ----------------
void readSoil();
void readDHT();
void readPIR();

// ===================== SETUP =====================
void setup() {
  Serial.begin(9600);
  delay(2000);   // Allow sensors to stabilize

  pinMode(SOIL_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH);  // Motor OFF

  dht.begin();
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(1500L, readSoil);
  timer.setInterval(2500L, readDHT);
  timer.setInterval(500L, readPIR);
}

// ===================== SOIL MOISTURE =====================
void readSoil() {
  int raw = analogRead(SOIL_PIN);

  // 🔧 CALIBRATION (adjust after checking Serial Monitor)
  int moisture = map(raw, 820, 420, 0, 100);
  moisture = constrain(moisture, 0, 100);

  Serial.print("Soil Raw: ");
  Serial.print(raw);
  Serial.print("  Moisture(%): ");
  Serial.println(moisture);

  Blynk.virtualWrite(VPIN_SOIL, moisture);

  // ---------- AUTO MOTOR CONTROL ----------
  if (moisture <= 30 && relayState == HIGH) {
    relayState = LOW;   // Motor ON
    Serial.println("Motor ON (Soil Dry)");
  }
  else if (moisture >= 60 && relayState == LOW) {
    relayState = HIGH;  // Motor OFF
    Serial.println("Motor OFF (Soil Wet)");
  }

  digitalWrite(RELAY_PIN, relayState);
}

// ===================== DHT11 =====================
void readDHT() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read DHT sensor");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C  Humidity: ");
  Serial.println(humidity);

  Blynk.virtualWrite(VPIN_TEMP, temperature);
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity);
}

// ===================== PIR SENSOR =====================
void readPIR() {
  if (pirToggle != 1) {
    Blynk.virtualWrite(VPIN_PIR_LED, 0);
    return;
  }

  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH) {
    Serial.println("Motion Detected!");
    Blynk.virtualWrite(VPIN_PIR_LED, 255);
    Blynk.logEvent("pirmotion", "Warning: Motion Detected!");
  } else {
    Blynk.virtualWrite(VPIN_PIR_LED, 0);
  }
}

// ===================== PIR TOGGLE =====================
BLYNK_WRITE(VPIN_PIR_TOGGLE) {
  pirToggle = param.asInt();
}

// ===================== LOOP =====================
void loop() {
  Blynk.run();
  timer.run();
}
