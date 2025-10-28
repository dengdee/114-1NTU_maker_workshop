#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SD.h>
#include <SPI.h>

const char* ssid = "csie523";
const char* password = "MakeReality";
const char* host = "esp32-test-prs1.onrender.com";  // 去掉 https://
const int httpsPort = 443;
const char* urlPath = "/upload";

#define SD_CS 5
#define BOUNDARY "ESP32Boundary"

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 連接 WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected.");

  // 初始化 SD 卡
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD card init failed!");
    return;
  }

  File file = SD.open("/record.wav", FILE_READ);
  if (!file) {
    Serial.println("❌ Failed to open record.wav");
    return;
  }

  // 建立 HTTPS 連線
  WiFiClientSecure client;
  client.setInsecure(); // 忽略憑證（測試用）
  if (!client.connect(host, httpsPort)) {
    Serial.println("❌ Connection failed!");
    file.close();
    return;
  }

  // 組 multipart header & footer
  String head = "--" + String(BOUNDARY) + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"record.wav\"\r\nContent-Type: audio/wav\r\n\r\n";
  String tail = "\r\n--" + String(BOUNDARY) + "--\r\n";

  // 計算 Content-Length
  size_t contentLength = head.length() + file.size() + tail.length();

  // 送 HTTP POST 請求頭
  client.printf("POST %s HTTP/1.1\r\n", urlPath);
  client.printf("Host: %s\r\n", host);
  client.println("User-Agent: ESP32");
  client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", BOUNDARY);
  client.printf("Content-Length: %u\r\n\r\n", contentLength);

  // 送 head
  client.print(head);

  // 分段送檔案（避免一次讀完占用太多 RAM）
  const size_t bufSize = 512;
  uint8_t buf[bufSize];
  while (file.available()) {
    size_t len = file.read(buf, bufSize);
    client.write(buf, len);
  }

  // 送 tail
  client.print(tail);

  Serial.println("📡 Request sent, waiting response...");

  // 讀回應
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break; // headers 結束
  }

  String response;
  while (client.available()) {
    response += (char)client.read();
  }
  Serial.println("✅ Response:\n" + response);

  file.close();
}

void loop() {}
