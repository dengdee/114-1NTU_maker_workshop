#include <WiFi.h>
#include <WebServer.h>

#define LED_PIN 2

// 設定 AP 模式的 SSID 和密碼(8個字以上)
const char *ssid = "ESP32-LED-Control";
const char *password = "12345678";


//SAT
// const char *ssid = "csie523";
// const char *password = "MakeReality";


// 建立 WebServer 物件
WebServer server(80);

void handleRoot() {
  // HTML 網頁內容
  String html = "<h1>ESP32 LED Controller</h1>";
  html += "<button onclick=\"fetch('/led/on')\">Turn ON</button>";
  html += "<button onclick=\"fetch('/led/off')\">Turn OFF</button>";
  html += "<script>fetch('/status').then(res => res.text()).then(status => alert('Current Status: ' + status));</script>";
  
  // 發送 HTML
  server.send(200, "text/html", html);
}

void handleLEDOn() {
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/plain", "LED is ON");
}

void handleLEDOff() {
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "LED is OFF");
}

void handleStatus() {
  if (digitalRead(LED_PIN) == HIGH) {
    server.send(200, "text/plain", "ON");
  } else {
    server.send(200, "text/plain", "OFF");
  }
}

void setup() {
  Serial.begin(115200);

  // 設定 LED 腳位
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  

  //AP 模式
  WiFi.softAP(ssid, password);
  Serial.println("AP Mode Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  //SAT 模式
  // Serial.println("[INFO] Connecting to Wi-Fi...");
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("\n[INFO] Connected to Wi-Fi!");
  // Serial.print("[INFO] IP Address: ");
  // Serial.println(WiFi.localIP());
  
  

  // 設定網頁路由
  server.on("/", handleRoot);
  server.on("/led/on", handleLEDOn);
  server.on("/led/off", handleLEDOff);
  server.on("/status", handleStatus);

  // 啟動 WebServer
  server.begin();
  Serial.println("Web Server Started");
}

void loop() {
  // 處理客戶端請求
  server.handleClient();
}
