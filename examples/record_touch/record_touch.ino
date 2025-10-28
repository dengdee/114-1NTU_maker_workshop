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
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE 16
#define BUFFER_SIZE 1024
#define TOUCH_PIN 4
float Gain = 3;  //Increase the volume of the microphone input

File file;
uint8_t buffer[BUFFER_SIZE];
size_t bytesRead;
int touchValue = 0;


// ---------------- I2S Setup ----------------
void setupI2S() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = (i2s_bits_per_sample_t)BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
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

//
void recording() {
  Serial.println("Start recording...");
  file = SD.open("/record.wav", FILE_WRITE);
  if (!file) {
    Serial.println("Failed!");
    return;
  }
  writeWavHeader(file, SAMPLE_RATE, BITS_PER_SAMPLE, 0);
  while (touchValue < 40) {
    i2s_read(I2S_PORT, buffer, BUFFER_SIZE, &bytesRead, 0);
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
  // 更新 WAV header
  uint32_t numSamples = file.size() - 44;
  numSamples /= (BITS_PER_SAMPLE / 8);
  writeWavHeader(file, SAMPLE_RATE, BITS_PER_SAMPLE, numSamples);
  file.close();
  Serial.println("Saved as /record.wav");
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card init failed!");
    while (1)
      ;
  }
  Serial.println("SD card ready.");
  setupI2S();
}

// ---------------- Loop ----------------

void loop() {
  touchValue = touchRead(TOUCH_PIN);

  if (touchValue < 40) {
    // 按下開始錄音
    recording();
    delay(1000);
  }
}