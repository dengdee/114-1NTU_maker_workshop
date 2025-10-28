#include "config.h"
float Gain = 2;
File file;
uint8_t buffer[BUFFER_SIZE];
size_t bytesRead;
int touchValue = 0;
String tmp="test";

// -------------------- WIFI --------------------
const char* ssid = "LAPTOP-GDFAD7NL4364";
const char* password = "878p21}H";
const char* host = "esp32-test-prs1.onrender.com";  // 去掉 https://
const int httpsPort = 443;
const char* urlPath2 = "/tts";
const char* urlPath = "/upload";
const String apiKey = "AIzaSyDuBKdiK4b4WO6XuiAOZ9DsP1WinfGPfrs";  // 你的 Gemini API Key


// OLED 初始化
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, OLED_CLOCK, OLED_DATA);

// -------------------- I2S 初始化函式 --------------------
void setupI2SInput() {
    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = (i2s_bits_per_sample_t)BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .fixed_mclk = 0
    };

    const i2s_pin_config_t pin_config = {
        .mck_io_num = -1,
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_PORT_RX, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT_RX, &pin_config);
    i2s_zero_dma_buffer(I2S_PORT_RX);
}

void setupI2SOutput() {
    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = (i2s_bits_per_sample_t)BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .fixed_mclk = 0
    };

    const i2s_pin_config_t pin_config = {
        .mck_io_num = -1,
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_PORT_TX, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT_TX, &pin_config);
    i2s_zero_dma_buffer(I2S_PORT_TX);
}

void writeWavHeader(File &file, int sampleRate, int bitsPerSample, uint32_t numSamples) {
  int byteRate = sampleRate * bitsPerSample / 8;
  int dataSize = numSamples * bitsPerSample / 8;

  file.seek(0);  // 確保從檔案開頭寫
  file.write((const uint8_t *)"RIFF", 4);
  int chunkSize = dataSize + 36;
  file.write((const uint8_t *)&chunkSize, 4);
  file.write((const uint8_t *)"WAVEfmt ", 8);

  int subchunk1Size = 16;
  short audioFormat = 1;
  short numChannels = 1;
  file.write((const uint8_t *)&subchunk1Size, 4);
  file.write((const uint8_t *)&audioFormat, 2);
  file.write((const uint8_t *)&numChannels, 2);
  file.write((const uint8_t *)&sampleRate, 4);
  file.write((const uint8_t *)&byteRate, 4);

  short blockAlign = bitsPerSample / 8;
  file.write((const uint8_t *)&blockAlign, 2);
  file.write((const uint8_t *)&bitsPerSample, 2);

  file.write((const uint8_t *)"data", 4);
  file.write((const uint8_t *)&dataSize, 4);
}

void recording() {
  Serial.println("Start recording...");
  showWords("Start recording...");
  // 刪除舊檔案（如果存在）
  if (SD.exists("/record.wav")) {
    SD.remove("/record.wav");
    Serial.println("Old /record.wav removed.");
    showWords("Old /record.wav removed.");
  }
  file = SD.open("/record.wav", FILE_WRITE);
  if (!file) {
    Serial.println("Failed!");
    showWords("Failed!");
    return;
  }
  writeWavHeader(file, SAMPLE_RATE, BITS_PER_SAMPLE, 0);
  while (touchValue < 40) {
    i2s_read(I2S_PORT_RX, buffer, BUFFER_SIZE, &bytesRead, 0);
    // 放大音量
    for (size_t i = 0; i < bytesRead; i += 2) {
      int16_t sample = buffer[i] | (buffer[i + 1] << 8);
      int32_t amplified = (int32_t)(sample * Gain);
      if (amplified > 32767) amplified = 32767;
      if (amplified < -32768) amplified = -32768;
      buffer[i] = amplified & 0xFF;
      buffer[i + 1] = (amplified >> 8) & 0xFF;
    }
    file.write(buffer, bytesRead);
    touchValue = touchRead(TOUCH_PIN);
  }
  // 放開停止錄音
  Serial.println("Stop recording...");
  showWords("Stop recording...");
  // 更新 WAV header
  uint32_t numSamples = file.size() - 44;
  numSamples /= (BITS_PER_SAMPLE / 8);
  writeWavHeader(file, SAMPLE_RATE, BITS_PER_SAMPLE, numSamples);
  file.close();
  Serial.println("Saved as /record.wav");
  showWords("Saved as /record.wav");
}

void playAudio() {
  File file = SD.open("/speech.wav");
  if (!file) {
    Serial.println("Failed to open file for reading!");
    showWords("Failed to open file for reading!");
    return;
  }

  // 跳過 WAV 標頭 (44 bytes)
  file.seek(44);
  uint8_t buffer[BUFFER_SIZE];
  size_t bytesWritten;

  digitalWrite(DAC_SD, HIGH);
  Serial.println("Playing...");
  showWords("Playing...");
  while (file.available()) {
    size_t bytesRead = file.read(buffer, BUFFER_SIZE);
    i2s_write(I2S_PORT_TX, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
  }

  file.close();
  digitalWrite(DAC_SD, LOW);
  Serial.println("Playback finished.");
  showWords("Playback finished.");
}

void showWords(const char *str) {
  u8g2.clearBuffer();                                  // 清空畫面 buffer
  u8g2.setFont(u8g2_font_6x10_tf);          // 中文字型              
  const uint8_t lineHeight = u8g2.getMaxCharHeight();  // 每行高度
  const uint8_t screenWidth = 128;                     // OLED 寬度
  const uint8_t screenHeight = 64;                     // OLED 高度
  uint8_t x = 0;
  uint8_t y = lineHeight;

  const char *p = str;
  char line[128];  // 暫存一行文字
  uint8_t lineLen = 0;

  while (*p) {
    line[lineLen++] = *p;

    // 計算目前 line 的寬度
    line[lineLen] = '\0';
    int16_t w = u8g2.getStrWidth(line);

    if (w > screenWidth || *p == '\n') {
      // 超出寬度或遇到換行符號就換行
      line[lineLen - 1] = '\0';  // 移除最後一個字（或換行符號）
      u8g2.setCursor(x, y);
      u8g2.print(line);
      y += lineHeight;

      if (y > screenHeight) break;  // 超出螢幕範圍停止

      lineLen = 0;                           // 清空暫存行
      if (*p != '\n') line[lineLen++] = *p;  // 將超出字放入下一行
    }

    p++;
  }

  // 印出最後一行
  if (lineLen > 0 && y <= screenHeight) {
    line[lineLen] = '\0';
    u8g2.setCursor(x, y);
    u8g2.print(line);
  }

  u8g2.sendBuffer();  // 將 buffer 傳到 OLED
}

void send_wav() {

  File file = SD.open("/record.wav", FILE_READ);
  if (!file) {
    Serial.println("❌ Failed to open record.wav");
    showWords("Failed to open record.wav");
    return;
  }
  // 建立 HTTPS 連線
  WiFiClientSecure client;
  client.setInsecure();  // 忽略憑證（測試用）
  if (!client.connect(host, httpsPort)) {
    Serial.println("❌ Connection failed!");
    showWords("Connection failed!");
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
  showWords("Request sent, waiting response...");

  // 讀回應
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;  // headers 結束
  }

  String response;
  while (client.available()) {
    response += (char)client.read();
  }
  // 解析 JSON
  DynamicJsonDocument doc(512);  // 記得根據 JSON 大小調整
  DeserializationError error = deserializeJson(doc, response);

  int idx = response.indexOf('{');
  if (idx >= 0) {
    String jsonStr = response.substring(idx);
    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, jsonStr) == DeserializationError::Ok) {
      const char* text = doc["text"];
      tmp=String(text);
      Serial.println("🎯 Text only: " + tmp);
      showWords(text);
    }
  }

  file.close();
  delay(1000);
}

void send_to_llm() {
  String userText = tmp;
  HTTPClient http;
  http.setReuse(false);  // 每次請求都重新連線
  String url = "https://generativelanguage.googleapis.com/v1/models/gemini-2.0-flash:generateContent?key=" + apiKey;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // 組成 JSON body
  // String payload = "{ \"contents\": [{ \"parts\": [{ \"text\": \"" + userText + "\" }] }] }";
  String payload = "{ \"contents\": [{ \"parts\": [{ \"text\": \"" + userText + "\" }] }], \"generationConfig\": { \"maxOutputTokens\": 50 } }";
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
          tmp=String(generatedText);
          Serial.println("Generated Text:");
          Serial.println(generatedText);
          showWords(generatedText);
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
}

void downloadTTS() {
  WiFiClientSecure client;
  client.setInsecure();  // 測試用，跳過證書驗證

  HTTPClient https;
  String url = String("https://") + host + urlPath2;
  https.begin(client, url);
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String payload = "text=" + tmp + "&lang=en";
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

