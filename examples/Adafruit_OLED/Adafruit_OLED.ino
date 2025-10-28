//reference:https://www.youtube.com/watch?v=r-bhlCULPX0

#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WebServer server(80);

const char* ssid = "csie523"; 
const char* password = "MakeReality";

bool pixelBuffer[SCREEN_WIDTH][SCREEN_HEIGHT];

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html><html>
  <head>
    <title>ESP32 OLED Touch Paint</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { text-align: center; font-family: Arial; }
      canvas { border: 2px solid #000; touch-action: none; }
      button { margin-top: 10px; padding: 10px 20px; font-size: 16px; }
    </style>
  </head>
  <body>
    <h2>ESP32 OLED Touch Painting</h2>
    <canvas id="canvas" width="128" height="64"></canvas><br>
    <button onclick="clearDisplay()"> Clear Display</button>
    
    <script>
      const canvas = document.getElementById('canvas');
      const ctx = canvas.getContext('2d');
      let painting = false;

      function draw(x, y) {
        ctx.fillStyle = "black";
        ctx.fillRect(x, y, 1, 1);
        fetch(`/draw?x=${x}&y=${y}`);
      }

      function getPos(e) {
        const rect = canvas.getBoundingClientRect();
        let x, y;
        if (e.touches) {
          x = e.touches[0].clientX - rect.left;
          y = e.touches[0].clientY - rect.top;
        } else {
          x = e.clientX - rect.left;
          y = e.clientY - rect.top;
        }
        return { x: Math.floor(x), y: Math.floor(y) };
      }

      canvas.addEventListener("mousedown", (e) => { painting = true; });
      canvas.addEventListener("mouseup", () => { painting = false; });
      canvas.addEventListener("mousemove", (e) => {
        if (!painting) return;
        const { x, y } = getPos(e);
        draw(x, y);
      });

      canvas.addEventListener("touchstart", (e) => {
        painting = true;
        const { x, y } = getPos(e);
        draw(x, y);
        e.preventDefault();
      }, { passive: false });

      canvas.addEventListener("touchmove", (e) => {
        if (!painting) return;
        const { x, y } = getPos(e);
        draw(x, y);
        e.preventDefault();
      }, { passive: false });

      canvas.addEventListener("touchend", () => { painting = false; });

      function clearDisplay() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        fetch("/clear");
      }
    </script>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleDraw() {
  if (server.hasArg("x") && server.hasArg("y")) {
    int x = server.arg("x").toInt();
    int y = server.arg("y").toInt();
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
      pixelBuffer[x][y] = true;
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleClear() {
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      pixelBuffer[x][y] = false;
    }
  }
  display.clearDisplay();
  display.display();
  server.send(200, "text/plain", "Cleared");
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.clearDisplay();
  display.display();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/draw", handleDraw);
  server.on("/clear", handleClear);
  server.begin();
}

void loop() {
  server.handleClient();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 100) {
    lastUpdate = millis();
    display.clearDisplay();
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      for (int y = 0; y < SCREEN_HEIGHT; y++) {
        if (pixelBuffer[x][y]) {
          display.drawPixel(x, y, SSD1306_WHITE);
        }
      }
    }
    display.display();
  }
}
