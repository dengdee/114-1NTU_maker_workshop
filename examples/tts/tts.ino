#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>

const char* ssid = "LAPTOP-GDFAD7NL4364";
const char* password = "878p21}H";

const char* host = "esp32-test-prs1.onrender.com";
const int httpsPort = 443;
const char* urlPath = "/tts";

#define SD_CS 5
#define WAV_FILE "/speech.wav"

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 連接 WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // 初始化 SD 卡
  if (!SD.begin(SD_CS)) {
    Serial.println("SD init failed!");
    return;
  }
  Serial.println("SD init done");

  // 下載 TTS
  downloadTTS("how are you? I'm fine thankyou and you?");
}

void downloadTTS(const char* str2) {
  WiFiClientSecure client;
  client.setInsecure();  // 測試用，跳過證書驗證

  HTTPClient https;
  String url = String("https://") + host + urlPath;
  https.begin(client, url);
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String payload = "text=" + String(str2) + "&lang=en";
  int httpCode = https.POST(payload);

  if (httpCode == 200) {
    Serial.println("TTS request successful, downloading WAV...");

    File file = SD.open(WAV_FILE, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file on SD");
      https.end();
      return;
    }

    // 使用 getStream()，分段讀取
    WiFiClient* stream = https.getStreamPtr();
    int len = https.getSize();  // 回傳資料長度
    Serial.println("Content length: " + String(len));

    uint8_t buffer[512];
    int bytesRead = 0;

    while (https.connected() && (bytesRead < len)) {
      size_t sizeAvailable = stream->available();
      if (sizeAvailable) {
        int c = stream->readBytes(buffer, ((sizeAvailable > sizeof(buffer)) ? sizeof(buffer) : sizeAvailable));
        file.write(buffer, c);
        bytesRead += c;
      }
      delay(1);
    }
    file.close();
    Serial.println("WAV saved to SD: " + String(WAV_FILE));
  }
}

void loop() {

}
