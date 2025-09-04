// ----------- Blynk Credentials -----------
#define BLYNK_TEMPLATE_ID "TMPL35D1G0Chs"
#define BLYNK_TEMPLATE_NAME "Temperature logging system"
#define BLYNK_AUTH_TOKEN "7PMPvmXe6xx7eukfLqq2fLiNBEpsAis6"

// ----------- Libraries -----------
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <BlynkSimpleEsp32.h>
#include <time.h>  // For NTP

// ----------- WiFi Credentials -----------
char ssid[] = "DESKTOP"; 
char pass[] = "123456789";

// ----------- API Endpoint -----------
const char* serverName = "http://172.20.10.2:5000/api/logs";  // my local IP for the lapto

// ----------- DHT Sensor Setup -----------
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ----------- Setup Function -----------
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Initialize Blynk + WiFi (handles WiFi connection itself)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Setup NTP time
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for time sync...");
  while (time(nullptr) < 100000) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nTime synced");
}

// ----------- Function to Get Timestamp -----------
String getFormattedTime() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}

// ----------- Main Loop -----------
void loop() {
  Blynk.run();

  static unsigned long lastReadTime = 0;
  const unsigned long readInterval = 10000; // Every 10 seconds

  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    String timestamp = getFormattedTime();

    // Log to Serial
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidity: ");
    Serial.print(humidity);
    Serial.print(" % | Time: ");
    Serial.println(timestamp);

    // Send to Blynk
    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);

    // Send to Local API
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      String jsonPayload = "{";
      jsonPayload += "\"temperature\":" + String(temperature) + ",";
      jsonPayload += "\"humidity\":" + String(humidity) + ",";
      jsonPayload += "\"timestamp\":\"" + timestamp + "\"";
      jsonPayload += "}";

      int httpResponseCode = http.POST(jsonPayload);
      if (httpResponseCode > 0) {
        Serial.print("API POST Success, Code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("API POST Failed: ");
        Serial.println(http.errorToString(httpResponseCode));
      }
      http.end();
    } else {
      Serial.println("WiFi disconnected");
    }
  }
}