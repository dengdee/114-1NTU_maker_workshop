// for SD card
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define SD_CS 5

//for inmp441 mic
#include "driver/i2s.h"
#define I2S_WS 15
#define I2S_SCK 14
#define I2S_SD 32
#define I2S_PORT_RX I2S_NUM_0
#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE 16
#define BUFFER_SIZE 1024
#define TOUCH_PIN 4
float Gain = 2;  //Increase the volume of the microphone input

//for max98357A
#define I2S_DOUT 25
#define I2S_LRC 26
#define I2S_BCLK 27
#define DAC_SD 33
#define I2S_PORT_TX I2S_NUM_1


File file;
uint8_t buffer[BUFFER_SIZE];
size_t bytesRead;
int touchValue = 0;

//for oled
#include <Wire.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21);


// -------------------- I2S 初始化 --------------------
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

// ---------------- WAV Header ----------------
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

// -------------------- 錄音 --------------------
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

// -------------------- 播放 --------------------
void playAudio() {
  File file = SD.open("/record.wav");
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
  u8g2.setFont(u8g2_font_unifont_t_chinese2);          // 中文字型              
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

// -------------------- 主程式 --------------------
void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.enableUTF8Print();                      // 允許顯示 UTF-8（中文字需要）
  pinMode(DAC_SD, OUTPUT);
  digitalWrite(DAC_SD, LOW);  // 靜音
  delay(1000);
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
    setupI2SOutput();
    delay(1000);
    playAudio();
    i2s_driver_uninstall(I2S_PORT_TX);
    delay(2000);
    setupI2SInput();
  }
}
