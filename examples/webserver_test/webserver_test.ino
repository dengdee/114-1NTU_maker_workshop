#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "csie523"; //you can change to your own wifi
const char* password = "MakeReality";
const char* serverUrl = "https://esp32-test-prs1.onrender.com/test"; //change to your own server url

// 建立 HTTP 連線
HTTPClient http;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 連接 WiFi
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
  http.begin(serverUrl);
  Serial.println("Sending GET request...");
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("Response code: %d\n", httpCode);
    String payload = http.getString();
    Serial.println("Server response:");
    Serial.println(payload);
  } else {
    Serial.printf("Connection failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  delay(15000);
}
