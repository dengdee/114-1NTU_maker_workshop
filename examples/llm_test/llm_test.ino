#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "csie523";
const char* password = "MakeReality";
const String apiKey = "";  // 你的 Gemini API Key

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 連接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println("請在 Serial Monitor 輸入要查詢的文字，然後按 Enter");
}

void loop() {
  if (Serial.available()) {
    String userText = Serial.readStringUntil('\n');
    userText.trim();
    if (userText.length() == 0) return;

    Serial.println("Sending: " + userText);

    HTTPClient http;
    http.setReuse(false);  // 每次請求都重新連線
    String url = "https://generativelanguage.googleapis.com/v1/models/gemini-2.0-flash:generateContent?key=" + apiKey;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // 組成 JSON body
    // String payload = "{ \"contents\": [{ \"parts\": [{ \"text\": \"" + userText + "\" }] }] }";
    String payload = "{ \"contents\": [{ \"parts\": [{ \"text\": \"" + userText + "\" }] }], \"generationConfig\": { \"maxOutputTokens\": 300 } }";
    int httpCode = http.POST(payload);
    if (httpCode > 0) {
      String response = http.getString();
      // Serial.println("Raw Response:");
      // Serial.println(response);  // 若要 debug 可以印出完整 JSON

      // 解析 JSON
      DynamicJsonDocument doc(8192);  // Gemini response 有時候比較長
      DeserializationError error = deserializeJson(doc, response);
      if (!error) {
        if (doc.containsKey("candidates")) {
          const char* generatedText = doc["candidates"][0]["content"]["parts"][0]["text"];
          if (generatedText) {
            Serial.println("Generated Text:");
            Serial.println(generatedText);
          } else {
            Serial.println("⚠️ 回應中沒有生成文字");
          }
        } else {
          Serial.println("⚠️ 沒有找到 candidates");
        }
      } else {
        Serial.println("JSON parse error");
      }
    } else {
      Serial.print("HTTP request failed, code: ");
      Serial.println(httpCode);
    }

    http.end();
    Serial.println("\n請輸入下一個文字:");
  }
}
