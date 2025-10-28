
#include "config.h"

// -------------------- 主程式 --------------------
void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.enableUTF8Print();  // 允許顯示 UTF-8（中文字需要）
  pinMode(DAC_SD, OUTPUT);
  digitalWrite(DAC_SD, LOW);  // 靜音


  // 連接 WiFi
  showWords("Connecting wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected.");
  showWords("WiFi connected.");

  showWords("Initializing SD card...");
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    showWords("SD card initialization failed!");
    return;
  }
  Serial.println("SD card ready.");
  showWords("SD card ready.");
  setupI2SInput();
}

void loop() {

  touchValue = touchRead(TOUCH_PIN);

  if (touchValue < 40) {
    // 按下開始錄音
    recording();
    i2s_driver_uninstall(I2S_PORT_RX);

    send_wav();
    delay(1000);
    Serial.println(tmp);
    send_to_llm();
    delay(1000);
    downloadTTS();
    setupI2SOutput();
    delay(1000);
    playAudio();
    i2s_driver_uninstall(I2S_PORT_TX);
    delay(2000);
    setupI2SInput();
  }
}
